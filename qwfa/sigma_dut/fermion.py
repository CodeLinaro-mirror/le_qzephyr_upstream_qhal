#!/usr/bin/env python3
#===============================================================================
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear
#===============================================================================

import time
import re
import serial
import threading
import inspect
import logging
import logging.handlers
from cli_helper import CLI_Helper

logger = logging.getLogger('mylogger.fermion')
logger.info("[DUT_LOG] Fermion test\r\n")

class Fermion_Command:
    def __init__(self,test_params):
        self.dutCrashTag  = 0
        self.ser = CLI_Helper(test_params.serial_port)
        self.dev_name = test_params.interface
        self.psk = ""
        self.security = 0
        self.ap_ssid = ""
        self.ap_channel = ""
        self.ping_count = 0

    def getDutCrashStatus(self):
        tag = self.ser.checkdutCrashTag()
        return tag

    def clearDutBuffer(self):
        dutOutput = self.ser.readSerialbuffer()
        return dutOutput

    def runCLIQuick(self,command):
        dutOutput = self.ser.writeSerialQuick(command)
        #wait for serial output complete
        time.sleep(0.05)

    def initReset(self):
        dutOutput = self.ser.writeSerial("platform reboot")
        return dutOutput

    def setApWireless(self, ssid, channel):
        self.ap_ssid = ssid
        self.ap_channel = channel
        # dutOutput = self.ser.writeSerial("wlan SetOperatingMode ap")
        # dutOutput = self.ser.writeSerial("wlan Set11nHTCap ht20")
        # dutOutput = self.ser.writeSerial("wlan SetChannel 1")
        return None

    def enableWireless(self):
        pass

    def wlanScan(self):
        dutOutput = self.ser.writeSerial("wifi scan")
        return dutOutput

    def setDevice(self):
        pass

    def workMode(self):
        pass

    def powerSave(self):
        logger.info("{} called.".format(inspect.currentframe().f_code.co_name))
        # dutOutput = self.ser.writeSerial("wlan EnableGTX 0")
        # dutOutput = self.ser.writeSerial("wlan EnableLPL 0")
        # return dutOutput

    def setPowerSave(self,params):
        logger.info("{} called.".format(inspect.currentframe().f_code.co_name))
        # if (re.search("on",params,re.I)):
        #     dutOutput = self.ser.writeSerial("wlan SetPowerMode 1")
        # else:
        #     dutOutput = self.ser.writeSerial("wlan SetPowerMode 0")
        # return dutOutput

    def disableDeepSleep(self):
        logger.info("{} called.".format(inspect.currentframe().f_code.co_name))
        # dutOutput = self.ser.writeSerial("LP DeepSleep Disable")
        # return dutOutput

    def enableAggr(self):
        dutOutput = self.ser.writeSerial("qwifi set_aggregation 0xff 0xff")
        return dutOutput

    def getVersion(self):
        dutOutput = self.ser.writeSerial("platform version")
        pattern   = "CRM Number:\s*(.*)"
        dutOutput    = self.getMatchingInfo(pattern,dutOutput,1)
        return dutOutput

    def getBssid(self):
        dutOutput = self.ser.writeSerial("wifi status")
        pattern   = "BSSID:\s*(([0-9A-F]{1,2}[:]){5}[0-9A-F]{1,2})"
        bssid    = self.getMatchingInfo(pattern,dutOutput,1)
        return bssid

    def getMacAddr(self, interface = None):
        if interface:
            mac_command = "net iface {}".format(interface)
        else:
            mac_command = "net iface"
        dutOutput = self.ser.writeSerial(mac_command)
        pattern   = "Link addr :\s+(([0-9A-F]{1,2}[:]){5}[0-9A-F]{1,2})"
        MacAddr    = self.getMatchingInfo(pattern,dutOutput, 1)
        return MacAddr

    def getDutConnectStatus(self):
        dutOutput = self.ser.writeSerial("wifi status")
        if re.search("State: COMPLETED",dutOutput):
            return 1
        else:
            return 0

    def getIPconfig(self,wirelessInterface):
        dutOutput = self.ser.writeSerial("net iface")
        ip_and_mask_pattern = "IPv4 unicast addresses.*?\n\s*((?:\d{1,3}\.){3}\d{1,3})/((?:\d{1,3}\.){3}\d{1,3})"
        gateway_pattern = "IPv4 gateway :\s*((?:\d{1,3}\.){3}\d{1,3})"
        ip         = self.getMatchingInfo(ip_and_mask_pattern,dutOutput,1)
        netmask    = self.getMatchingInfo(ip_and_mask_pattern,dutOutput,2)
        gateway    = self.getMatchingInfo(gateway_pattern,dutOutput,1)
        dns        = None

        self.dutIp = ip
        return (ip, netmask, dns, gateway)

    def trafficReset(self):
        stop_rx_command = "wificert download stop"
        stop_tx_command = "wificert upload stop"
        self.ser.writeSerialQuick(stop_rx_command)
        self.ser.writeSerialQuick(stop_tx_command)
        #wait for the RX/TX traffic result.
        time.sleep(1.5)
        dutOutput = self.ser.readSerialbuffer()
        return dutOutput

    def stop_rx_traffic(self):
        reset_traffic = "wificert download stop"
        self.ser.writeSerialQuick(reset_traffic)
        #wait for the RX/TX traffic result.
        time.sleep(1.5)
        dutOutput = self.ser.readSerialbuffer()
        return dutOutput

    def setPrivatekey(self,psk):
        self.psk = psk
        return None

    def setEnterpriseParms(self, eapMethod, username, password):
        logger.info("{} called.".format(inspect.currentframe().f_code.co_name))
        # if re.match("TTLS",eapMethod,re.I):
        #     dutOutput = self.ser.writeSerial("wlan SetWpaCertParameters TTLS-MSCHAPV2 ioeDevice %s %s" %(username,password))
        # elif re.match("PEAP",eapMethod,re.I):
        #     dutOutput = self.ser.writeSerial("wlan SetWpaCertParameters PEAP-MSCHAPV2 ioeDevice %s %s" %(username,password))
        # elif re.match("TLS",eapMethod,re.I):
        #     dutOutput = self.ser.writeSerial("wlan SetWpaCertParameters TLS %s %s %s 0 rootCA wifiuser 0 0 0x1006b" %(username,username,password))
        #     #rootCA is calist and wifiuser is certificate,should modify by qlan
        # return dutOutput

    def setSecurity(self, encpType, keymgmttype, saeType):
        dutEncpType = ""
        if (re.match('^TKIP$',encpType,re.I)):
            dutEncpType = "TKIP"
        elif (re.match('^AES-CCMP$',encpType,re.I)):
            dutEncpType = "CCMP"
        elif (re.match('^AES-CCMP-128$',encpType,re.I)):
            dutEncpType = "CCMP"
        if dutEncpType == "":
            if "WPA2" in keymgmttype:
                dutEncpType = "CCMP"
            elif "WPA" in keymgmttype:
                dutEncpType = "TKIP"

        dutkeymgmttype = ""
        if (re.match('^WPA2$',keymgmttype,re.I)):
            dutkeymgmttype = "WPA2"
        elif (re.match('^WPA$',keymgmttype,re.I)):
            dutkeymgmttype = "WPA"
        elif (re.match('^WPA2-PSK$',keymgmttype,re.I)):
            dutkeymgmttype = "WPA2"
        elif (re.match('^WPA-PSK$',keymgmttype,re.I)):
            dutkeymgmttype = "WPA"
        else:
            dutkeymgmttype = ""

        if (re.match('^SAE$',saeType,re.I)):
            dutkeymgmttype = "SAE"
        elif (re.match('^PSK-SAE$',saeType,re.I)):
            dutkeymgmttype = "SAE_WPA2"

        security_map = {
            ("", ""):               0,   # None
            ("CCMP", "WPA2"):       1,   # WPA2-PSK
            ("CCMP-256", "WPA2"):   2,   # WPA2-PSK-256
            ("CCMP", "SAE"):        3,   # SAE (default to HNP)
            ("CCMP", "SAE-H2E"):    4,   # SAE-H2E
            ("CCMP", "SAE-AUTO"):   5,   # SAE-AUTO
            ("CCMP", "WAPI"):       6,   # WAPI
            ("CCMP", "EAP-TLS"):    7,   # EAP-TLS
            ("WEP", "WEP"):         8,   # WEP
            ("TKIP", "WPA"):        9,   # WPA-PSK
            ("TKIP", "WPA-AUTO"):   10,  # WPA-Auto-Personal
            ("CCMP", "DPP"):        11,  # DPP
            ("CCMP", "EAP-PEAP-MSCHAPv2"): 12,
            ("CCMP", "EAP-PEAP-GTC"):      13,
            ("CCMP", "EAP-TTLS-MSCHAPv2"): 14,
            ("CCMP", "EAP-PEAP-TLS"):      15,
            ("CCMP", "SAE_WPA2"):   20,  # SAE-EXT-KEY / PSK-SAE
        }

        self.security = security_map.get((dutEncpType, dutkeymgmttype), 0)

        return None

    def setEnterpriseSecurity(self,encpType,keymgmttype):
        logger.info("{} called.".format(inspect.currentframe().f_code.co_name))
        # if (re.match('^TKIP$',encpType,re.I)):
        #     dutEncpType = "TKIP"
        # elif (re.match('^AES-CCMP$',encpType,re.I)):
        #     dutEncpType = "CCMP"
        # else:
        #     dutEncpType = ""
        # logger.info( "keymgmttype in the params is %s !!!" %keymgmttype)

        # if (re.match('^WPA2$',keymgmttype,re.I)):
        #     dutkeymgmttype = "WPA2CERT"
        # elif (re.match('^WPA$',keymgmttype,re.I)):
        #     dutkeymgmttype = "WPACERT"
        # else:
        #     dutkeymgmttype = ""

        # dutOutput = self.ser.writeSerial("wlan SetWpaParameters %s %s %s" %(dutkeymgmttype,dutEncpType,dutEncpType))
        # return dutOutput

    def setSaeGroup(self,groupID):
        logger.info("{} called.".format(inspect.currentframe().f_code.co_name))
        # dutOutput = self.ser.writeSerial("wlan SetSaeGroups %s" %groupID)
        # return dutOutput

    def pmfMode(self,pmf):
        logger.info("{} called.".format(inspect.currentframe().f_code.co_name))
        # num = ""
        # if re.search('require',pmf,re.I):
        #     num = 2
        # elif re.search('optional',pmf,re.I):
        #     num = 1
        # elif re.search('disable',pmf,re.I):
        #     num = 0
        # else:
        #     return None
        # dutOutput = self.ser.writeSerial("wlan SetPmfMode %s" %num)
        # return dutOutput

    def reAssocAp(self, reassoc_dict):
        self.disconnectAp()
        if "pmfTag" in reassoc_dict.keys():
            self.pmfMode(reassoc_dict['pmfTag'])
        if "psk" in reassoc_dict.keys():
            self.setPrivatekey(reassoc_dict['psk'])
        if "saeType" in reassoc_dict.keys():
            self.setSecurity(reassoc_dict['encpType'],reassoc_dict['keymgmttype'],reassoc_dict['saeType'])
        if "groupID" in reassoc_dict.keys():
            self.setSaeGroup(reassoc_dict['groupID'])
        if "ssid" in reassoc_dict.keys():
            self.connectAp(reassoc_dict['ssid'])

    def ap_enable(self):
        if self.security != 0:
            ap_command = "wifi ap enable -k {} -s {} -c {}".format(self.security, self.ap_ssid, self.ap_channel)
        else:
            ap_command = "wifi ap enable -s {} -c {}".format(self.ap_ssid, self.ap_channel)

        dutOutput = self.ser.writeSerial(ap_command)
        return dutOutput

    def connectAp(self, ssid):
        ssid = ssid.strip()
        if self.security != 0:
            connect_command = "wifi connect -s {} -p {} -k {}".format(ssid, self.psk, self.security)
        else:
            connect_command = "wifi connect -s {}".format(ssid)
        dutOutput = self.ser.writeSerial(connect_command)
        return dutOutput

    def disconnectAp(self):
        dutOutput = self.ser.writeSerial("wifi disconnect")

    def setUapsd(self,params):
        logger.info("{} called.".format(inspect.currentframe().f_code.co_name))
        # dutOutput = self.ser.writeSerial("wlan setSTAUapsd %s" %params)
        # return dutOutput

    def sendUapsd(self,params,readTag):
        dutOutput = ""
        if readTag:
            dutOutput = self.ser.writeSerial("Net uapsdc send %s" %params)
        else:
            self.ser.writeSerialQuick("Net uapsdc send %s" %params)
        return dutOutput

    def connectEnterpriseAp(self,eapMethod,ssid,username,password,encpType,keymgmttype):
        self.setEnterpriseSecurity(encpType,keymgmttype)
        self.setEnterpriseParms(eapMethod,username,password)
        dutOutput = self.ser.writeSerial("wlan Connect %s" %ssid)
        return dutOutput

    def initDhcp(self,interface):
        dhcpc_command = "net dhcpv4 client start {}".format(interface)
        dutOutput = self.ser.writeSerial(dhcpc_command)
        return dutOutput

    def setStaticIp(self, interface, ip, mask):
        #m = re.compile('(([0-9]{1,3}[.]){3}[0-9]{1,3})',gateway)
        #m.replace('(([0-9]{1,3}[.]){3}[0-9]{1,3})','(([0-9]{1,3}[.]){3})')
        ip_config = "net ipv4 add {} {} {}".format(interface, ip, mask)
        dutOutput = self.ser.writeSerial(ip_config)
        return dutOutput

    def set_ap_dhcp_pool(self, interface, ip):
        dhcps_command = "net dhcpv4 server start {} {}".format(interface, ip)
        #m = re.compile('(([0-9]{1,3}[.]){3}[0-9]{1,3})',gateway)
        #m.replace('(([0-9]{1,3}[.]){3}[0-9]{1,3})','(([0-9]{1,3}[.]){3})')
        dutOutput = self.ser.writeSerial(dhcps_command)
        return dutOutput

    def sendPing(self, dest_ip, count, framesize):
        self.ping_count = count
        ping_command = "net ping {} -c {} -s {}".format(dest_ip, count, framesize)
        dutOutput = self.ser.writeSerialQuick(ping_command)
        return dutOutput

    def ping_success_times(self, dutOutput):
        pingSucessTimes = self.getMatchingInfo('Received packets = (\d+)',dutOutput,1)

        if pingSucessTimes == "":
            pingSucessTimes =  dutOutput.count("bytes from")

        return pingSucessTimes

    def ping_send_times(self,dutOutput):
        return self.ping_count

    def configTrafficCommand(self,uccCommand):
        #traffic_agent_config,profile,Multicast,direction,send,destination,224.0.0.5,destinationPort,223,source,192.165.100.90,sourcePort,223,duration,90,trafficClass,BestEffort,payloadSize,350,frameRate,50
        #traffic_agent_config,profile,Multicast,direction,receive,sourcePort,223,destinationPort,223,destination,224.0.0.5
        #traffic_agent_config,profile,IPTV,direction,receive,source,192.165.100.30,sourcePort,4600,destinationPort,4600
        #traffic_agent_config,profile,uapsd,direction,receive,source,192.165.100.30,sourcePort,4600,destinationPort,4600
        profile    = self.getMatchingInfo('profile,(\w+),',uccCommand,1)
        direction    = self.getMatchingInfo('direction,(\w+),',uccCommand,1)
        destinationPort    = self.getMatchingInfo('destinationPort,(\d+),',uccCommand,1)
        source    = self.getMatchingInfo('source,(([0-9][.]){3}[0-9]{1,3}),',uccCommand,1)
        sourcePort    = self.getMatchingInfo('sourcePort,(\d+),',uccCommand,1)
        if re.search('send',direction,re.I):
            duration    = self.getMatchingInfo('duration,(\d+),',uccCommand,1)
            trafficClass    = self.getMatchingInfo('trafficClass,(\w+),',uccCommand,1)
            payloadSize    = self.getMatchingInfo('payloadSize,(\d+),',uccCommand,1)
            frameRate    = self.getMatchingInfo('frameRate,(.*)(\s)',uccCommand,1)
            destination    = self.getMatchingInfo('destination,(([0-9]{1,3}[.]){3}[0-9]{1,3}),?',uccCommand,1)
        else:
            duration       = ""
            trafficClass   = ""
            payloadSize    = ""
            frameRate      = ""

        commandLine = ''
        port = ''
        time = ''
        protocol = ""
        if profile:
            if   re.search('File_Transfer',profile,re.I):
                protocol = ''
            elif re.search('Multicast',profile,re.I):
                protocol = ''
            elif re.search('IPTV',profile,re.I):
                protocol = ''
            elif re.search('Transaction',profile,re.I):
                protocol = '-e '
            elif re.search('Start_Sync',profile,re.I):
                protocol = ''

        if frameRate:
            frameRate = frameRate.strip()
            if re.match("0", frameRate,re.I):
                frameRate = ""
            else:
                frameRate = str(int(frameRate) * 1000)

        traffcTc = '0x00'
        if trafficClass:
            if   re.search('Voice',trafficClass,re.I):
                traffcTc = '0xE0'
            if   re.search('Video',trafficClass,re.I):
                traffcTc = '0xA0'
            if   re.search('BestEffort',trafficClass,re.I):
                traffcTc = '0x00'
            if   re.search('Background',trafficClass,re.I):
                traffcTc = '0x32'

        if re.search('Receive',direction,re.I):
            if  re.search('Multicast',profile,re.I):
                destination    = self.getMatchingInfo('destination,(([0-9]{1,3}[.]){3}[0-9]{1,3}),?',uccCommand,1)
                dutIp = self.dutIp
                # commandLine = "WiFiCert udp -s "+ protocol + " -p " + sourcePort + " -B " + destination
                commandLine = "wificert download {} {} {}".format(protocol, sourcePort, destination)
            else:
                # commandLine = "WiFiCert udp -s "+ protocol + " -p " + sourcePort
                commandLine = "wificert download {} {}".format(protocol, sourcePort)
            port = sourcePort
        elif  re.search('Send',direction,re.I):
            # commandLine = "WiFiCert udp -c " + destination + protocol + " -p " + destinationPort + " -l " + payloadSize + " -i " + str(delay) + " -S " + traffcTc + " -t " + str(duration)
            commandLine = "wificert upload -S {} {} {} {} {} {}".format(traffcTc, destination, destinationPort, duration, payloadSize, frameRate)
            port = destinationPort
            time = duration
        return (commandLine,port,time)

    def analysisSendResults(self,runID,port,dutOutput):
        # Sent 1756 packets, 2458400 bytes to 192.165.100.30 5601
        # Sent 14423 packets, 20192200 bytes to 192.165.100.30 5600
        tx_pkts = 0
        rx_pkts = 0
        tx_bytes = 0
        rx_bytes = 0

        tx_pkts_pattern = "Num packets:\s*(\d+)"
        result = re.search(tx_pkts_pattern, dutOutput)
        if result is None:
            return ValueList
        tx_pkts = int(result.group(1))

        tx_bytes_pattern = "Num Bytes:\s*(\d+)"
        result = re.search(tx_bytes_pattern, dutOutput)
        if result is None:
            return ValueList
        tx_bytes = int(result.group(1))

        return [str(runID), str(tx_pkts), str(rx_pkts), str(tx_bytes), str(rx_bytes), "0"]

    def analysisRecvResults(self,runID,port,dutOutput):
        tx_pkts = 0
        rx_pkts = 0
        tx_bytes = 0
        rx_bytes = 0

        rx_pkts_pattern = "Num packets:\s*(\d+)"
        result = re.search(rx_pkts_pattern, dutOutput)
        if result is None:
            return ValueList
        rx_pkts = int(result.group(1))

        rx_bytes_pattern = "Num Bytes:\s*(\d+)"
        result = re.search(rx_bytes_pattern, dutOutput)
        if result is None:
            return ValueList
        rx_bytes = int(result.group(1))

        return [str(runID), str(tx_pkts), str(rx_pkts), str(tx_bytes), str(rx_bytes), "0"]

    def getMatchingInfo(self,pattern,dutOutput,feedbackLocation = 0):
        matchInfo = ""
        m = re.search(pattern,dutOutput,re.I)
        if m is not None:
            matchInfo = m.group(feedbackLocation)
        return matchInfo

