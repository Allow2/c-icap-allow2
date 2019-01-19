
# Introduction

c-icap-allow2 has been built to enable parents to control access to internet and certain websites for their children based on
* the type of day (ie: a school day, a weekend, a sick day, a holiday, etc),
* the time of day,
* the amount of activity time a child has remaining (ie: no more than 2 hours of gaming, or 3 hours of internet, etc),
* any existing restrictions (ie: a child is currently banned from using the internet such as when on timeout or grounded),
* any particular extensions that have been granted
* and lots more.

For more information on the capabilities of Allow2, create a free account at https://app.allow2.com and try it for yourself.

In order to control access for a child, c-icap-allow2 is supplied as a plugin service for the c-icap server.
This service acts as a proxy filter for any router that supports icap (and was designed to work with Squid, but could be any icap-enabled proxy).

## Children

A key requirement to using Allow2 is being able to identify the child using any particular resource (gaming console, PC, mobile device, tablet, web browser, etc) at a point in time in order
to be able to apply the appropriate rules.

To achieve this on a network device, there are a couple of options:
1. Use some authentication specific to the child (ie a user account on the proxy server), or
2. associate a specific device to a specific user (allow2 provides a couple of options to achieve this).

c-icap-allow2 can be configured to associate a proxy user id to a specific child, and it is advised to enable this to make things simpler and more robust.
however, it has a fallback to allow you to configure a permanent device association with a child.

In lieu of either method being capable of identifying a child accessing the web, you can configure c-icap-allow2 to default to blocking or allowing the access.

You can also configure specific devices or proxy accounts as "always allow" or "always block" - this provides a way to bypass the rules for parents or other users.


# Dependencies

* unix build tools: automake, autoconf
* OpenCV - Used for all image and video classification.
* C-ICAP - This is a c-icap module. It requires c-icap development libraries to be compiled. It is run through C-ICAP.
* An ICAP enabled proxy which can do ACLs based on HTTP reply headers (Squid is one such proxy.)

# Installation

There are 3 parts required to make use of c-icap-allow2.
* install c-icap and c-icap-allow2 on a suitable server (which could be your home server, an OpenWRT router, a raspberry pi, a virtual server, etc)
* connect it to a suitable icap-enabled proxy (which could be built in to your particular router, or could be Squid on a server, or maybe all on OpenWRT)
* correctly configure your network/firewalls/clients/etc according to your requirements (outside the scope of this document).

The installation steps below are written as if you were installing Squid, c-icap and c-icap-allow2 on an OpenWRT router.
An alternate configuration is to set up c-icap and c-icap-allow2 on a separate server and point any icap capable proxy on any router to this icap server.

## Install Squid 3.1+ on your OpenWRT box

Either use the OpenWRT gui to install:
* squid
* luci-app-squid (if you want to use luci to configure squid)
* squid-mod-cachemgr (is this required?)

Or install via commandline:
```sh
opkg install squid luci-app-squid squid-mod-cachemgr
```

## Install c-icap

General installation instructions: http://c-icap.sourceforge.net/install.html

You can install c-icap on OpenWRT using this guide: TBA
Alternately install on another host or create VM and install there

## Install c-icap-allow2

?????

Add the Allow2 service to c-icap.conf:

```text
Service allow2 srv_allow2.so
```

(Recommended) Set "RemoteProxyUsers on" in c-icap.conf to use the proxy user as the identifier for the child:

```text
RemoteProxyUsers on
```

You then configure c-icap-allow2 using the mappings for users and IP addresses to either a child account


##Configure Squid to use c-icap

add the following to the squid configuration (use 127.0.0.1 if c-icap is on the same router, or the correct IP if it's on a different host):

```text
# enable icap
icap_enable on
icap_send_client_ip on
icap_send_client_username on
icap_client_username_encode off
icap_client_username_header X-Authenticated-User
icap_preview_enable on
icap_preview_size 1024

icap_service service_req reqmod_precache bypass=1 icap://127.0.0.1:1344/request
adaptation_access service_req allow all

icap_service service_resp respmod_precache bypass=0 icap://127.0.0.1:1344/response
adaptation_access service_resp allow all
```

## Configure Users for Squid (Recommended)

Although you can also assign users by IP address, it is recommended you set up separate proxy users for each child. Then they can have the proxy set individually on each of their accounts/devices/systems
and that will be much better for shared devices and also if devices change IP address (if they set it manually or something).

If proxy users are used, their creds will be sent to the icap service and take precedence. If not using that, or the user is not authenticated, then the mapping to children will fall back to IP addresses.
You can also set a default child to fallback to for ALL checks (allowing you to default to a child UNLESS they have a proxy account. And you can also map some proxy accounts as "always allow" for any parents or other users.

To set up basic username/password auth for squid:
(see https://stackoverflow.com/questions/3297196/how-to-set-up-a-squid-proxy-with-basic-username-and-password-authentication)

```text
auth_param basic program /usr/lib/squid3/basic_ncsa_auth /etc/squid3/passwords
auth_param basic realm proxy
acl authenticated proxy_auth REQUIRED
http_access allow authenticated
```

Then you can set up users like this:
```sh
sudo htpasswd -c /etc/squid3/passwords username_you_like
```

And don't forget to restart the squid service after adding/changing and removing credentials.
```sh
service squid3 restart
```

## Testing the Installation

View logs on Squid and run the c-icap using:

```sh
c-icap -N -D -d 10
```