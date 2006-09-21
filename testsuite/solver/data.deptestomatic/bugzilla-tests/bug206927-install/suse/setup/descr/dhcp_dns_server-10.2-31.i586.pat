# openSUSE Patterns 10.2-31.i586 -- (c) 2006 SUSE LINUX Products GmbH
# generated on Tue Sep 19 13:20:53 UTC 2006

=Ver: 5.0

=Pat:  dhcp_dns_server 10.2 31 i586

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

=Ico: yast-dns-server

=Sum: DHCP and DNS Server
=Sum.bg: DHCP и DNS сървър
=Sum.cs: DHCP a DNS server
=Sum.da: DHCP og DNS server
=Sum.de: DHCP- und DNS-Server
=Sum.es: Servidor DHCP y DNS
=Sum.fi: DHCP- ja DNS-palvelin
=Sum.fr: Serveur DNS et DHCP
=Sum.hu: DHCP- és DNS-kiszolgáló
=Sum.it: Server DHCP e DNS
=Sum.ja: DHCPおよびDNSサーバ
=Sum.km: ម៉ាស៊ីន​បម្រើ DHCP និង DNS
=Sum.nb: DHCP- og DNS-server
=Sum.nl: DHCP- en DNS-server
=Sum.pl: Serwer DHCP i DNS
=Sum.pt: Servidor DHCP e DNS
=Sum.pt_BR: Servidor DHCP e DNS
=Sum.ru: Серверы протокол динамического выбора хост-машины (DHCP) и доменная система имен (DNS)
=Sum.sk: Server DHCP a DNS
=Sum.sv: DHCP- och DNS-server
=Sum.uk: Сервер DHCP та DNS
=Sum.zh_CN: DHCP 和 DNS 服务器
=Sum.zh_TW: DHCP 及 DNS 伺服器

+Des.cs:
Nastavení DHCP serveru, který se postará o automatické síťové nastavení pracovních stanic ve vaší síti. Součásti je také DNS server, který se postará o správné rozpoznávání jmen staniv síti.
-Des.cs:
+Des.da:
Sæt DHCP (Dynimic Host Configuration Protocol) og/eller DNS (Domain Name System)  op på maskinen. DHCP lader klienter på nettet få IP adresser automatisk og DNS sørger for at man kan bruge domænenavne i stedet for IP numre til at tilgå maskiner med.
-Des.da:
+Des.de:
Hiermit können Sie den Server für das Dynamic Host Configuration Protocol (DHCP) einrichten, das Konfigurationsparameter für Client-Computer bereitstellt, um sie in ein Netzwerk zu integrieren. Außerdem können Domain Name System-(DNS-)Dienste bereitgestellt werden, mit denen Informationen zu Domänennamen, insbesondere die IP-Adresse, zugestellt werden können.
-Des.de:
+Des:
Set up the server for the Dynamic Host Configuration Protocol (DHCP) that provides configuration parameters to client computers to integrate them into a network. Also provide Domain Name System (DNS) services to deliver information associated with domain names, in particular, the IP address.
-Des:
+Des.es:
Instala el servidor para el protocolo de configuración dinámica de host (DHCP) que ofrece los parámetros de configuración de los equipos cliente para integrarlos en una red. También ofrece los servicios del sistema de nombres de dominio (DNS) para proporcionar información asociada a los nombre de dominios, en particular a las direcciones IP.
-Des.es:
+Des.fi:
Aseta DHCP-palvelin (Dynamic Host Configuration Protocol), joka tarjoaa määritysasetukset asiakaskoneille liittääkseen ne verkkoon. Tarjoa myös DNS-palvelun (Domain Name System) välittääkseen tietoja liittyen toimialuenimiin, kuten IP-osoitteen.
-Des.fi:
+Des.fr:
Ce serveur est configuré pour prendre en charge le protocole de configuration d'hôte dynamique (DHCP). Ce protocole fournit aux ordinateurs clients les paramètres de configuration nécessaires pour les intégrer à un réseau. Ce serveur offre également des services de système de noms de domaine (DNS) pour livrer les informations associées aux noms de domaine, en particulier l'adresse IP.
-Des.fr:
+Des.hu:
DHCP-kiszolgáló beállítása, amelyik hálózati konfigurációs paramétereket szolgáltat a kliensszámítógépek számára. Ezenfelül DNS-szolgáltatások biztosítása a tartománynevekkel kapcsolatos információ, elsősorban az IP-cím kiszolgálására.
-Des.hu:
+Des.it:
Configurare il server per il protocollo DHCP (Dynamic Host Configuration Protocol) che fornisce i parametri di configurazione per consentire l'integrazione dei computer client in una rete. Specificare inoltre i servizi DNS (Domain Name System) per la distribuzione delle informazioni associate ai nomi di dominio, in particolare l'indirizzo IP.
-Des.it:
+Des.ja:
クライアントコンピュータに構成パラメータを提供してネットワークへ統合する動的ホスト構成プロトコル(DHCP)用のサーバをセットアップします。 ドメイン名に関連した情報、具体的にはIPアドレスを提供するためのドメイン名システム(DNS)サービスも用意します。
-Des.ja:
+Des.km:
រៀបចំ​ម៉ាស៊ីន​បម្រើ​សម្រាប់​ពិធីការ​កំណត់​រចនាសម្ព័ន្ធ​ម៉ាស៊ីន​ថាមវន្ត (DHCP) ដែល​ផ្ដល់​នូវ​ប៉ារ៉ាមែត្រ​កំណត់​រចនាសម្ព័ន្ធ​ទៅ​ឲ្យ​ម៉ាស៊ីន​ភ្ញៀវ ដើម្បី​រួមបញ្ចូល​ពួក​វា​ទៅ​ក្នុង​បណ្ដាញ​មួយ ។ វា​ក៏​ផ្ដល់​ផង​ដែរ​នូវ​សេវា​ប្រព័ន្ធ​ឈ្មោះ​ដែន (DNS) ដើម្បី​ចែកចាយ​ព័ត៌មាន​ដែល​ទាក់ទង​ជាមួយ​ឈ្មោះ​ដែន ជា​ពិសេស​គឺ​អាសយដ្ឋាន IP ។
-Des.km:
+Des.nb:
Sett opp en DHCP-server (Dynamic Host Configuration Protocol)som forsyner klientmaskiner med konfigurasjonsparametere for å integrere dem i et nettverk, og som også utfører DNS-tjenester (Domain Name System) ved å formidle informasjon i forbindelse med domenenavn, spesielt IP-adresser.
-Des.nb:
+Des.nl:
Stel de server in voor de DHCP die configuratie-parameters levert aan client-computers om deze te integreren in een netwerk. Lever ook DNS-services aan voor het aanleveren van informatie geassocieerd aan domeinnamen zoals het IP-adres.
-Des.nl:
+Des.pl:
Serwer DHCP (Dynamic Host Configuration Protocol) udostępnia komputerom klienckim parametry konfiguracyjne, dzięki którym mogą one połączyć się z siecią. Serwer DNS (Domain Name System) dostarcza informacje związane z nazwami domenowymi, w szczególności adresy IP.
-Des.pl:
+Des.pt:
Configura o servidor para o Dynamic Host Configuration Protocol (DHCP) que fornece parâmetros de configuração a computadores cliente, para os integrar numa rede. Também fornece serviços de Domain Name System (DNS) para facilitar informação associada com nomes de domínio, mais em particular os endereços IP.
-Des.pt:
+Des.pt_BR:
Configura o servidor para o DHCP (Dynamic Host Configuration Protocol), que fornece parâmetros de configuração a computadores clientes para integrá-los a uma rede. Também oferece serviços de Domain Name System (DNS) para fornecer informações associadas a nomes de domínio, especificamente o endereço IP.
-Des.pt_BR:
+Des.zh_CN:
设置动态主机配置协议（DHCP）的服务器，该服务器为客户机提供配置参数，以便将它们集成到网络中。 同时还提供了域名系统（DNS）服务，以交付与域名（尤其是 IP 地址）相关的信息。
-Des.zh_CN:
+Des.zh_TW:
可設定「動態主機組態協定」 (DHCP) 的伺服器，提供組態參數給用戶端電腦以將其整合到網路中。也提供「領域名稱系統」(DNS) 服務以傳遞與領域名稱相關的資訊，特別是 IP 位址。
-Des.zh_TW:

+Req:
basesystem
-Req:

=Vis: true

=Ord: 3060

+Prq:
bind
bind-chrootenv
dhcp
dhcp-relay
dhcp-server
dhcp-tools
-Prq:
+Prc:
bind-doc
dhcp6
-Prc:

