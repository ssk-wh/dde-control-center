// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef BRIGHTNESSWIDGET_H
#define BRIGHTNESSWIDGET_H

#include "interface/namespace.h"
#include "widgets/titlelabel.h"

#include <DTipLabel>

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QSpacerItem;
class QVBoxLayout;
QT_END_NAMESPACE

namespace dcc {

namespace widgets {
class TipsLabel;
class SwitchWidget;
class SettingsGroup;
class TitledSliderItem;
} // namespace widgets

namespace display {
class Monitor;
class DisplayModel;
} // namespace display

} // namespace dcc

namespace DCC_NAMESPACE {

namespace display {

class BrightnessWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BrightnessWidget(QWidget *parent = 0);
    ~BrightnessWidget();

public:
    void setMode(dcc::display::DisplayModel *model);
    void setAdjustCCTmode(int mode);
    void showBrightness(dcc::display::Monitor *monitor = nullptr);

Q_SIGNALS:
    void requestSetMonitorBrightness(dcc::display::Monitor *monitor, const double brightness);
    void requestAmbientLightAdjustBrightness(const bool able);
    void requestSetColorTemperature(const int value);
    void requestSetMethodAdjustCCT(const int mode);

private:
    void addSlider();
    int colorTemperatureToValue(int kelvin);
    QString brightnessToTickInterval(const double tb) const;
    void setColorTemperatureVisible(bool visible);

private:
    dcc::display::DisplayModel *m_displayModel;

    QVBoxLayout *m_centralLayout;
    TitleLabel *m_brightnessTitle;
    QSpacerItem *m_autoLightSpacerItem;
    dcc::widgets::SwitchWidget *m_autoLightMode;
    QSpacerItem *m_colorSpacerItem;
    QWidget *m_tempratureColorWidget;
    TitleLabel *m_tempratureColorTitle;
    dcc::widgets::SwitchWidget *m_nightShift;
    DTK_WIDGET_NAMESPACE::DTipLabel *m_nightTips;
    dcc::widgets::SettingsGroup *m_settingsGroup;
    dcc::widgets::SwitchWidget *m_nightManual;
    dcc::widgets::TitledSliderItem *m_cctItem;
    QSpacerItem *m_nightShiftSpacerItem;
    QSpacerItem *m_nightTipsSpacerItem;
    QSpacerItem *m_nightManualSpacerItem;

    int m_miniScales = 0;
    QMap<dcc::display::Monitor *, QWidget *> m_monitorBrightnessMap;
};

} // namespace display

} // namespace DCC_NAMESPACE

#endif // BRIGHTNESSWIDGET_H
