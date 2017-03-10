#include "weatherrequest.h"
#include "networkutil.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QFile>
#include <QTimer>
#include <QProcess>
#include <QLocale>
#include <QSettings>

#include <QDomDocument>

static const QString WeatherServiceHost = "http://hualet.org:9898";
static const QString GeoNameServiceHost = "http://api.geonames.org";
static const QString GeoNameKey=  "change";

WeatherRequest::WeatherRequest(QObject *parent) :
    QObject(parent),
    m_retryTimer(new QTimer(this))
{
    qRegisterMetaType<City>();

    m_settings = new QSettings("deepin", "dcc-weather-plugin");
    m_city.geonameId = restoreGeoNameID();

    m_loader = new LoaderCity(this);
    m_manager = new QNetworkAccessManager(this);

    connect(m_loader, &LoaderCity::done, this, &WeatherRequest::setCity);
    connect(m_manager, &QNetworkAccessManager::finished, this, [](QNetworkReply *reply) { reply->deleteLater(); });

    m_retryTimer->setSingleShot(false);
    m_retryTimer->setInterval(5000);

    auto func = [this] {
        if (m_city.geonameId.isEmpty()) {
            m_loader->start();
        } else {
            refreshData();
        }
    };
    connect(m_retryTimer, &QTimer::timeout, this, func);

    func();
    m_retryTimer->start();
}

WeatherRequest::~WeatherRequest()
{

}

void WeatherRequest::setCity(const City &city)
{
    m_city = city;
    saveGeoNameID(m_city.geonameId);
    refreshData(true);
}

void WeatherRequest::processWeatherServiceReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    m_items.clear();

    QByteArray ba = reply->readAll();
    QJsonArray items = QJsonDocument::fromJson(ba).array();
    for (QJsonValue val : items) {
        QJsonObject obj = val.toObject();

        WeatherItem item;
        item.setName(obj["name"].toString().toLower());
        item.setDescription(obj["description"].toString());
        QDateTime dt; dt.setTime_t(obj["date"].toInt());
        item.setDate(dt.date());
        item.setTemperature(obj["temperatureMin"].toInt(), obj["temperatureMax"].toInt());

        m_items << item;
    }

    if (m_items.length()) {
        emit dataRefreshed(m_items);
        m_retryTimer->stop();
        m_lastRefreshTimestamp.restart();
    }
}

void WeatherRequest::processGeoNameIdReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    QByteArray ba = reply->readAll();

    QDomDocument domDocument;
    QString errorMsg;
    if (!domDocument.setContent(ba, false, &errorMsg)) {
        qDebug() << ba;
        qWarning() << "read xml content error! " << errorMsg;
    }

    QDomElement root = domDocument.documentElement();
    QDomElement geoname = root.firstChildElement("geoname");
    while (!geoname.isNull()) {
        QString name = geoname.firstChildElement("name").text();
        if (name.toLower() == m_city.name.toLower()) {
            QString geonameId = geoname.firstChildElement("geonameId").text();
            qDebug() << "got geoname id " << geonameId;
            saveGeoNameID(geonameId);
            requestWeatherForecast(geonameId);
            requestGeoNameInfo(geonameId);
            break;
        }
        geoname = geoname.nextSiblingElement("geoname");
    }
}

void WeatherRequest::processGeoNameInfoReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    QByteArray ba = reply->readAll();

    QDomDocument domDocument;
    QString errorMsg;
    if (!domDocument.setContent(ba, false, &errorMsg)) {
        qDebug() << ba;
        qWarning() << "read xml content error! " << errorMsg;
    }

    m_city.localizedName = m_city.name;
    QDomElement root = domDocument.documentElement();
    QDomElement name = root.firstChildElement("name");
    if (!name.text().isEmpty()) {
        m_city.localizedName = name.text();
        qDebug() << "got localized city name" << m_city.localizedName;
    }

    emit fetchLocalizedCityNameDone(m_city.localizedName);
}

void WeatherRequest::processSearchCityReply()
{
    QList<City> cities;

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    QByteArray ba = reply->readAll();

    QDomDocument domDocument;
    QString errorMsg;
    if (!domDocument.setContent(ba, false, &errorMsg)) {
        qDebug() << ba;
        qWarning() << "read xml content error! " << errorMsg;
    }

    m_city.localizedName = m_city.name;
    QDomElement root = domDocument.documentElement();
    QDomElement geoname = root.firstChildElement("geoname");
    while (!geoname.isNull()) {
        QString name = geoname.firstChildElement("name").text();
        QString geonameId = geoname.firstChildElement("geonameId").text();
        QString countryName = geoname.firstChildElement("countryName").text();

        cities.append(City{geonameId, countryName, "", name, name, 0, 0});

        geoname = geoname.nextSiblingElement("geoname");
    }

    emit searchCityDone(cities);
}

void WeatherRequest::saveGeoNameID(const QString &geonameId)
{
    m_settings->beginGroup("Location");
    m_settings->setValue("GeoNameID", geonameId);
    m_settings->endGroup();
}

QString WeatherRequest::restoreGeoNameID() const
{
    QString ret;

    m_settings->beginGroup("Location");
    ret = m_settings->value("GeoNameID", "").toString();
    m_settings->endGroup();

    return ret;
}

QString WeatherRequest::city() const
{
    return m_city.name;
}

QString WeatherRequest::localizedCityName() const
{
    return m_city.localizedName;
}

int WeatherRequest::count() const
{
    return m_items.count();
}

WeatherItem WeatherRequest::dayAt(int index)
{
    if ( index >= 0 && index < m_items.count() ) {
        return m_items[index];
    }

    return WeatherItem();
}

void WeatherRequest::refreshData(bool force)
{
    const int elapsed = m_lastRefreshTimestamp.elapsed();
    if ((elapsed >= 1000 * 60 * 15 || elapsed == 0 || force)) {
        m_retryTimer->start();

        City city = m_city;

        if (city.geonameId.isEmpty()) {
            QString geoNameIDUrl = QString("%1/extendedFindNearby?lat=%2&lng=%3&username=%4").arg(GeoNameServiceHost) \
                    .arg(city.latitude).arg(city.longitude).arg(GeoNameKey);
            QNetworkReply *reply = m_manager->get(QNetworkRequest(geoNameIDUrl));
            connect(reply, &QNetworkReply::finished, this, &WeatherRequest::processGeoNameIdReply);
        } else {
            requestWeatherForecast(city.geonameId);
            requestGeoNameInfo(city.geonameId);
        }
    }
}

void WeatherRequest::sendRefreshSignal()
{
    emit dataRefreshed(m_items);
}

void WeatherRequest::searchCity(const QString &input)
{
    qDebug() << "search city with input " << input;
    const QString lang = QLocale::system().name().split("_").at(0);
    QString searchCityUrl = QString("%1/search?q=%2&maxRows=10&username=%3&lang=%4").arg(GeoNameServiceHost) \
            .arg(input).arg(GeoNameKey).arg(lang);
    QNetworkReply *reply = m_manager->get(QNetworkRequest(searchCityUrl));
    connect(reply, &QNetworkReply::finished, this, &WeatherRequest::processSearchCityReply);
}

void WeatherRequest::requestWeatherForecast(const QString &geonameId)
{
    qDebug() << "request weather forecast " << geonameId;
    QString weatherUrl = QString("%1/forecast/%2").arg(WeatherServiceHost).arg(geonameId);
    QNetworkReply *reply = m_manager->get(QNetworkRequest(weatherUrl));
    connect(reply, &QNetworkReply::finished, this, &WeatherRequest::processWeatherServiceReply);
}

void WeatherRequest::requestGeoNameInfo(const QString &geonameId)
{
    qDebug() << "request geoname city info " << geonameId;
    const QString lang = QLocale::system().name().split("_").at(0);
    QString geoNameInfoUrl = QString("%1/get?geonameId=%2&username=%3&lang=%4").arg(GeoNameServiceHost) \
            .arg(geonameId).arg(GeoNameKey).arg(lang);
    QNetworkReply *reply = m_manager->get(QNetworkRequest(geoNameInfoUrl));
    connect(reply, &QNetworkReply::finished, this, &WeatherRequest::processGeoNameInfoReply);
}

LoaderCity::LoaderCity(QObject *parent)
    :QThread(parent)
{

}

void LoaderCity::run()
{
    m_city = NetworkUtil::city();

    emit done(m_city);
}

City LoaderCity::city() const
{
    return m_city;
}
