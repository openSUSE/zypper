# openSUSE Patterns 10.2-31.i586 -- (c) 2006 SUSE LINUX Products GmbH
# generated on Tue Sep 19 13:20:53 UTC 2006

=Ver: 5.0

=Pat:  directory_server 10.2 31 i586

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

=Ico: yast-ldap-server

=Sum: Directory Server (LDAP)
=Sum.cs: Adresářový server (LDAP)
=Sum.da: Opslagsserver (LDAP)
=Sum.de: Verzeichnisserver (LDAP)
=Sum.es: Servidor de directorios (LDAP)
=Sum.fi: Hakemistopalvelin (LDAP)
=Sum.fr: Serveur de répertoires (LDAP)
=Sum.hu: Címtárkiszolgáló (LDAP)
=Sum.it: Server di directory (LDAP)
=Sum.ja: ディレクトリサーバ(LDAP)
=Sum.km: ម៉ាស៊ីន​បម្រើ​ថត (LDAP)
=Sum.nb: Katalogserver (LDAP)
=Sum.nl: Directory-server (LDAP)
=Sum.pl: Serwer usług katalogowych (LDAP)
=Sum.pt: Servidor de Directório (LDAP)
=Sum.pt_BR: Servidor de Diretório (LDAP)
=Sum.uk: Сервер каталогів (LDAP)
=Sum.zh_CN: 目录服务器（LDAP）
=Sum.zh_TW: 目錄伺服器 (LDAP)

+Des.cs:
Nastavení adresářového serveru založeného na OpenLDAP a Kerberovi. LDAP (Lightweight Directory Access Protocol) je využíván k přístupu k online adresářovým službám. Protokol využívá TCP a lze jej používat přímo nebo jako prostředek k přístupu ke službám založeným na X.500.
-Des.cs:
+Des.da:
Konfigurér en opslagsserver med OpenLDAP og Kerberos. LDAP (Lightweight Directory Access Protocol) bruges til at tilgå online opslagstjenester. Tjenesten kører over TCP og kan  bruges til at tilgå enkeltstående LDAP opslagstjenester eller tilgå en opslagstjeneste med X.500 certifikater.
-Des.da:
+Des.de:
Richten Sie mit OpenLDAP und Kerberos einen Verzeichnisserver ein. LDAP (Lightweight Directory Access Protocol) wird für den Zugriff auf Online-Verzeichnisdienste verwendet. Er wird direkt über TCP ausgeführt und kann für den Zugriff auf einen eigenständigen LDAP-Verzeichnisdienst oder auf einen Verzeichnisdienst mit X.500-Backend verwendet werden.
-Des.de:
+Des:
Set up a directory server with OpenLDAP and Kerberos.  The Lightweight Directory Access Protocol (LDAP) is used to access online directory services. It runs directly over TCP and can be used to access a stand-alone LDAP directory service or to access a directory service that has an X.500 back-end.
-Des:
+Des.es:
Instala un servidor de directorios con OpenLDAP y Kerberos. El protocolo ligero de acceso al directorio (LDAP) se utiliza para acceder a servicios de directorio en línea. Se ejecuta directamente en TCP y se puede utilizar para acceder a un servicio independiente de directorios LDAP o a un servicio de directorios con un sistema de apoyo X.500.
-Des.es:
+Des.fi:
Aseta hakemistopalvelin käyttäen OpenLDAP:tä ja Kerberos:ta.  LDAP-palvelinta (Lightweight Directory Access Protocol) käytetään pääsyyn verkkohakemistopalveluihin. Se toimii suoraan TCP:n päällä ja sitä voidaan käyttää pääsyyn itsenäiseen LDAP-hakemistopalveluun tai pääsyyn hakemistopalveluun, jossa on X.500-tausta.
-Des.fi:
+Des.fr:
Permet de mettre en place un serveur de répertoires avec OpenLDAP et Kerberos. Le protocole d'accès de répertoires légers (LDAP) est utlisé pour accéder à des services de répertoires en ligne. Il s'exécute directement via TCP et peut être utilisé pour accéder à un service de répertoires LDAP autonome ou à un service de répertoires doté d'un arrière-plan X.500.
-Des.fr:
+Des.hu:
OpenLDAP és Kerberos szolgáltatásokat is nyújtó címtárkiszolgáló beállítása. Az online címtárszolgáltatások elérésére a Lightweight Directory Access Protocol (LDAP) szolgál. Az LDAP a TCP-re épülve működik és használható akár önálló LDAP címtárszolgáltatás, akár egy X.500 hátterű címtárszolgáltatás elérésére.
-Des.hu:
+Des.it:
Configurare un server di directory con OpenLDAP e Kerberos.  Il protocollo LDAP (Lightweight  Directory Access Protocol) consente di accedere ai servizi di directory online. Viene eseguito direttamente via TCP e può essere utilizzato per accedere a un servizio di directory LDAP stand-alone oppure per accedere a un servizio di directory dotato di un back-end X.500.
-Des.it:
+Des.ja:
OpenLDAPおよびKerberosを使用してディレクトリサーバをセットアップします。  Lightweight  Directory Access Protocol (LDAP)は、オンラインディレクトリサービスにアクセスするために使用します。 TCP上で直接実行し、スタンドアロンLDAPディレクトリサービスにアクセスしたりX.500バックエンドを持つディレクトリサービスにアクセスするために使用できます。
-Des.ja:
+Des.km:
រៀបចំ​ម៉ាស៊ីន​បម្រើ​ថត​ជាមួយ OpenLDAP និង Kerberos ។ ពិធីការ​ចូលដំណើរការ​ថត​ល្មមៗ (LDAP) ត្រូវ​បាន​ប្រើ​ដើម្បី​ចូលដំណើរការ​សេវា​ថត​លើ​បណ្ដាញ ។ វា​រត់​ដោយ​ផ្ទាល់​លើ TCP និង អាច​ត្រូវ​បាន​ប្រើ​ដើម្បី​ចូលដំណើរការ​សេវា​ថត LDAP ដំណើរការ​តែ​ឯង ដែល​មាន​កម្មវិធី X.500 នៅ​ពី​ក្រោយ ។
-Des.km:
+Des.nb:
Sett opp en katalogserver med OpenLDAP og Kerberos. LDAP (Lightweight Directory Access Protocol) benyttes for å få tilgang til elektroniske katalogtjenester. Den kjører direkte via TCP, og kan benyttes for å få tilgang til en frittstående LDAP-katalogtjeneste eller eller en X.500-katalogtjeneste.
-Des.nb:
+Des.nl:
Stel een directory-server in met OpenLDAP en Kerberos. LDAP wordt gebruikt voor de toegang tot online directory-services. Het draait rechtstreeks over TCP en kan worden gebruikt voor toegang tot een stand-alone LDAP-directoryservice of voor toegang tot een directoryservice dat een X.500 back-end heeft.
-Des.nl:
+Des.pl:
Serwer usług katalogowych opartych na OpenLDAP i Kerberos. Protokól LDAP używany jest jako metoda dostępu do usług katalogowych online. Działa bezpośrednio ponad TCP i można się dzięki niemu połączyć z samodzielną usługą katalogową LDAP albo z usługą katalogową opartą o mechanizm X.500.
-Des.pl:
+Des.pt:
Configura um servidor de directório com OpenLDAP e Kerberos. O Lightweight Directory Access Protocol (LDAP) é utilizado para aceder a serviços online de directório. Corre directamente sobre TCP e pode ser utilizado para aceder a um directório LDAP isolado ou para aceder a um serviço de directório que tem um back-end X.500.
-Des.pt:
+Des.pt_BR:
Configura um servidor de diretório com OpenLDAP e Kerberos.  O LDAP (Lightweight Directory Access Protocol) é usado para o acesso online a serviços de diretório. Ele é executado diretamente sobre o TCP e pode ser usado para o acesso a um serviço de diretório LDAP independente ou para o acesso a um serviço de diretório que tenha um back end X.500.
-Des.pt_BR:
+Des.zh_CN:
用 OpenLDAP 和 Kerberos 设置目录服务器。  轻量级目录访问协议（LDAP）用于访问在线目录服务。 它直接通过 TCP 运行，可用于访问独立的 LDAP目录服务，也可用于访问具有 X.500 后端的目录服务。
-Des.zh_CN:
+Des.zh_TW:
使用 OpenLDAP 與 Kerberos 設定目錄伺服器。「精簡目錄存取協定」(LDAP) 可用來存取線上目錄服務。它可直接在 TCP 上執行，並可用來存取獨立  LDAP 目錄服務或存取具有 X.500 後端的目錄服務。
-Des.zh_TW:

+Req:
basesystem
-Req:

=Vis: true

=Ord: 3070

+Prq:
nss_ldap
openldap2
pam_ldap
-Prq:
+Prc:
-Prc:

