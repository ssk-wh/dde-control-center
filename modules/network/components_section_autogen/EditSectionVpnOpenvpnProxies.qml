// This file is automatically generated, please don't edit manully.
import QtQuick 2.1
import Deepin.Widgets 1.0
import "../components"

BaseEditSection {
    id: sectionVpnOpenvpnProxies
    section: "vpn-openvpn-proxies"
    
    header.sourceComponent: EditDownArrowHeader{
        text: dsTr("VPN Proxies")
    }

    content.sourceComponent: Column { 
        EditLineComboBox {
            id: lineProxyType
            key: "proxy-type"
            text: dsTr("Proxy Type")
        }
        EditLineTextInput {
            id: lineProxyServer
            key: "proxy-server"
            text: dsTr("Server IP")
        }
        EditLineSpinner {
            id: lineProxyPort
            key: "proxy-port"
            text: dsTr("Port")
        }
        EditLineSwitchButton {
            id: lineProxyRetry
            key: "proxy-retry"
            text: dsTr("Retry Indefinitely When Failed")
        }
        EditLineTextInput {
            id: lineHttpProxyUsername
            key: "http-proxy-username"
            text: dsTr("User Name")
        }
        EditLinePasswordInput {
            id: lineHttpProxyPassword
            key: "http-proxy-password"
            text: dsTr("Password")
        }
        EditLinePasswordToggle {
            targetTextInput: lineHttpProxyPassword
        }
    }
}
