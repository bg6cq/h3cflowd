## Log header fields

Fields in a log header
```
Field Length (Bytes) Description 

Version 1 Version number of the log packet:
·     1—Version 1.0.
·     3—Version 3.0.
·     5—Version 5.0.
·     7—Version 7.0.

LogType 1 Log type:
·     4—NAT flow log.
·     5—NAT66 flow log.
·     7—AFT IPv6 to IPv4 log.
·     8—AFT IPv4 to IPv6 log.
·     11—LB NAT64 flow log.
·     12—LB NAT46 flow log.
·     13—LB NAT44 flow log.
·     14—LB NAT66 flow log.
·     15—RIR flow log.

Count 2 Number of flows recorded in the current packet. The value range is 1 to 100.

Second 4 Number of seconds from 1970-01-01 00:00:00 to the time when the packet was generated.

FlowSequence 4 Sequence number of the log packet, which equals to the count of log packets of all log types and versions.

Chassis 2 Number of chassis sent the log packet.

Slot 1 Number of slot sent the log packet.

Cpu 1 Number of CPU sent the log packet.
```


## NAT flow log 3.0 fields

Field Length (Bytes) Description
```
Protocol 1 Protocol number.

Operator 1 Reasons why a NAT flow log was generated:
·     0—Reserved.
·     1—Flow was ended normally.
·     2—Flow was aged out because of aging timer expiration.
·     3—Flow was aged out because of configuration change.
·     4—Flow was aged out because of insufficient resources.
·     5—Reserved.
·     6—Regular connectivity check record for the active flow.
·     7—Flow was deleted because a new flow was created when the flow table was full.
·     8—Flow was created.
·     FE—Other reasons.
·     10-FE-1—Reserved for future use.

IPVersion 1 IP packet version.

TosIPv4 1 ToS field of the IPv4 packet.

SourceIP 4 Source IP address before NAT.

SrcNatIP 4 Source IP address after NAT.

DestIP 4 Destination IP address before NAT.

DestNatIP 4 Destination IP address after NAT.

SrcPort 2 Source TCP/UDP port number before NAT.

SrcNatPort 2 Source TCP/UDP port number after NAT.

DestPort 2 Destination TCP/UDP port number before NAT.

DestNatPort 2 Destination TCP/UDP port number after NAT.

StartTime 4 Start time of the flow, in seconds. The value is counted from 1970/1/1 0:0.

EndTime 4 End time of the flow, in seconds. The value is counted from 1970/1/1 0:0.
This field is 0 when the Operator field is 6.

InTotalPkg 4 Number of packets received for the session.

InTotalByte 4 Number of bytes received for the session.

OutTotalPkg 4 Number of packets sent for the session.

OutTotalByte 4 Number of bytes sent for the session.

InVPNID 1 ID of the source VPN instance.

OutVPNID 1 ID of the destination VPN instance.

vSystemID 2 vSystem ID.

AppID 4 Application protocol ID.

Reserved3 4 Reserved field.
```


## NAT66 flow log 3.0 fields

Field Length (Bytes) Description

```
Protocol 1 Protocol number.

Operator 1 Reasons why a NAT flow log was generated:
·     0—Reserved.
·     1—Flow was ended normally.
·     2—Flow was aged out because of aging timer expiration.
·     3—Flow was aged out because of configuration change.
·     4—Flow was aged out because of insufficient resources.
·     5—Reserved.
·     6—Regular connectivity check record for the active flow.
·     7—Flow was deleted because a new flow was created when the flow table was full.
·     8—Flow was created.
·     FE—Other reasons.
·     10-FE-1—Reserved for future use.

IPVersion 1 IP packet version.

Traffic class 1 Traffic Class field of the IPv6 packet.

SourceIP 16 Source IPv6 address before NAT.

DestIP 16 Destination IPv6 address before NAT.

SrcPort 2 Source TCP/UDP port number before NAT.

DestPort 2 Destination TCP/UDP port number before NAT.

StartTime 4 Start time of the flow, in seconds. The value is counted from 1970/1/1 0:0.

EndTime 4 End time of the flow, in seconds. The value is counted from 1970/1/1 0:0.
This field is 0 when the Operator field is 6.

InTotalPkg 4 Number of packets received for the session.

InTotalByte 4 Number of bytes received for the session.

OutTotalPkg 4 Number of packets sent for the session.

OutTotalByte 4 Number of bytes sent for the session.

InVPNID 2 ID of the source VPN instance.

OutVPNID 2 ID of the destination VPN instance.

ContextID 4 ID of the context for the session.

AppID 4 Application protocol ID.
```
