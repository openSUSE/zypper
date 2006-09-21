# openSUSE Patterns 10.2-31.i586 -- (c) 2006 SUSE LINUX Products GmbH
# generated on Tue Sep 19 13:20:54 UTC 2006

=Ver: 5.0

=Pat:  gateway_server 10.2 31 i586

=Cat.cs: Primární funkce
=Cat.da: Primære funktioner
=Cat.de: Primäre Funktionen
=Cat: Primary Functions
=Cat.es: Funciones principales
=Cat.fi: Ensisijaiset toiminnot
=Cat.fr: Fonctions principales
=Cat.hu: Elsődleges funkciók
=Cat.it: Funzioni principali
=Cat.ja: 基本機能
=Cat.km: អនុគមន៍​ចម្បង
=Cat.nb: Primærfunksjoner
=Cat.nl: Primaire functies
=Cat.pl: Funkcje podstawowe
=Cat.pt: Funções Principais
=Cat.pt_BR: Funções Principais
=Cat.uk: Головні функції
=Cat.zh_CN: 主要功能
=Cat.zh_TW: 主要功能

=Ico: yast-dsl

=Sum: Internet Gateway
=Sum.cs: Internetová brána
=Sum.da: Internet gateway
=Sum.de: Internet-Gateway
=Sum.es: Gateway de Internet
=Sum.fi: Internet-yhdyskäytävä
=Sum.fr: Passerelle Internet
=Sum.hu: Internetes átjáró
=Sum.it: Gateway Internet
=Sum.ja: インターネットゲートウェイ
=Sum.km: ផ្លូវ​ចេញ​ចូល​អ៊ីនធឺណិត
=Sum.nb: Internettsystemport
=Sum.nl: Internet-gateway
=Sum.pl: Brama internetowa
=Sum.pt: Gateway de Internet
=Sum.pt_BR: Gateway de Internet
=Sum.sv: Internet-gateway
=Sum.uk: Інтернет шлюз
=Sum.zh_CN: 因特网网关
=Sum.zh_TW: 網際網路閘道

+Des.cs:
Nastavení proxy, firewallu a brány pro připojení k Internetu. Součástí je také VPN (virtual private network).
-Des.cs:
+Des.da:
Sæt en proxy-, firewall og/eller gateway-server op. Der er også mulighed for at sætte virtuelle private netværk (VPN) op.
-Des.da:
+Des.de:
Hiermit können Sie einen Proxy-, Firewall- und Gateway-Server einrichten. Außerdem kann ein VPN-Gateway (Virtual Private Network) bereitgestellt werden.
-Des.de:
+Des:
Set up a proxy, firewall, and gateway server. Also provide a virtual private network (VPN) gateway.
-Des:
+Des.es:
Instala un servidor alterno (proxy), un cortafuegos y un gateway. Proporciona también un gateway de red privada virtual (VPN).
-Des.es:
+Des.fi:
Aseta välityspalvelin, palomuuri ja yhdyskäytäväpalvelin. Tarjoaa myös virtuaalisen yksityisen verkon (VPN) yhdyskäytävän.
-Des.fi:
+Des.fr:
Permet de mettre en place un serveur proxy, pare-feu et passerelle. Fournit également une passerelle pour réseau privé virtuel (VPN).
-Des.fr:
+Des.hu:
Proxy-, tűzfal- és átjárókiszolgáló beállítása. Szolgálhat virtuális magánhálózati (VPN-) átjáróként is.
-Des.hu:
+Des.it:
Impostare un server proxy, firewall e gateway. Specificare anche un gateway VPN (virtual private network).
-Des.it:
+Des.ja:
プロキシ、ファイアウォール、およびゲートウェイサーバをセットアップします。 仮想プライベートネットワーク(VPN)ゲートウェイも用意します。
-Des.ja:
+Des.km:
រៀបចំ​ម៉ាស៊ីន​បម្រើ​ប្រូកស៊ី ជញ្ជាំងភ្លើង និង ផ្លូវ​ចេញចូល ។ វា​ក៏​ផ្ដល់​ផង​ដែរ​នូវ​ផ្លូវ​ចេញចូល​បណ្ដាញ​ឯកជន​និម្មិត (VPN) ។
-Des.km:
+Des.nb:
Sett opp en mellomserver, brannmur og systemport som også fungerer som systemport for et virtuelt privat nettverk (VPN).
-Des.nb:
+Des.nl:
Stel een proxy-, firewall- en gateway-server in. Lever ook een VPN-gateway aan.
-Des.nl:
+Des.pl:
Konfigurowany będzie serwer pośredniczący, zapora sieciowa i serwer bramki. Dodatkowo zainstalowana zostanie brama wirtualnej sieci prywatnej (VPN).
-Des.pl:
+Des.pt:
Configura um servidor de proxy, firewall e gateway. Fornece também uma gateway de rede privada virtual (VPN).
-Des.pt:
+Des.pt_BR:
Configura um servidor proxy, de firewall e de gateway. Também fornece um gateway de VPN (virtual private network).
-Des.pt_BR:
+Des.uk:
Налаштувати сервер проксі, фаєрвол та шлюз. Також може надавати шлюз VPN (віртуальної приватної мережі).
-Des.uk:
+Des.zh_CN:
设置代理、防火墙和网关服务器。 同时提供虚拟的私用网络（VPN）网关。
-Des.zh_CN:
+Des.zh_TW:
設定代辦程式、防火牆及閘道伺服器。也會提供虛擬私密網路 (VPN) 閘道。
-Des.zh_TW:

+Req:
basesystem
-Req:

=Vis: true

=Ord: 3050

+Prc:
squid
squidGuard
ipsec-tools
quagga
radvd
rarpd
fetchmail
ddclient
ethereal
-Prc:

