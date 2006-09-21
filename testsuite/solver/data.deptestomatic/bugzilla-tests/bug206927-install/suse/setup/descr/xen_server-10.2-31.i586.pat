# openSUSE Patterns 10.2-31.i586 -- (c) 2006 SUSE LINUX Products GmbH
# generated on Tue Sep 19 13:21:02 UTC 2006

=Ver: 5.0

=Pat:  xen_server 10.2 31 i586

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

=Ico: yast-uml

=Sum: Xen Virtual Machine Host Server
=Sum.cs: Hostitelský systém pro Xen
=Sum.da: Xen virtuel maskine server
=Sum.de: Hostserver für Xen Virtual Machine
=Sum.es: Servidor host de máquina virtual Xen
=Sum.fi: Isäntäpalvelin Xen-virtuaalikoneille
=Sum.fr: Serveur hôte pour machine virtuelle Xen
=Sum.hu: Xen virtuálisgép-kiszolgáló
=Sum.it: Server host per computer virtuali Xen
=Sum.ja: Xen仮想マシンホストサーバ
=Sum.km: ម៉ាស៊ីន​បម្រើ​ម៉ាស៊ីន​របស់​ម៉ាស៊ីន​និម្មិត Xen
=Sum.nb: Xen-server for virtuell maskin
=Sum.nl: Xen Virtual Machine Host Server
=Sum.pl: Serwer komputerów wirtualnych Xen
=Sum.pt: Servidor Anfitrião de Máquina Virtual Xen
=Sum.pt_BR: Servidor Host de Máquina Virtual XEN
=Sum.uk: Сервер-вмістилище віртуальної машини Xen
=Sum.zh_CN: Xen 虚拟机器主管服务器
=Sum.zh_TW: Xen 虛擬機器主機伺服器

+Des.cs:
Nastavení a správa virtuálních počítačů na jednom fyzickém systému.
Zahrnuje:
 * XEN monitor virtuálního stroje
 * XEN nástroje monitoru virtuálního stroje a dokumentaci
 * XEN linuxové jádro
 * Moduly programu YaST pro instalaci a konfiguraci virtuálních systémů
-Des.cs:
+Des.da:
Konfigurér en server til at opsætte, styre og monitorere virtuelle maskiner på en enkelt fysisk maskine. Indeholder:
 * XEN virtuel maskine monitor
 * XEN virtuel maskine monitor værktøjer og dokumentation
 * XEN-tilpasset Linux kerne for den fysiske server
 * YaST moduler til at installere og opsætte virtuelle systemer
-Des.da:
+Des.de:
Richten Sie einen Server zur Konfiguration, Verwaltung und Überwachung von Virtual Machines auf einem einzelnen physischen Computer ein. Dazu gehört:
 * XEN Virtual Machine Monitor
  * Tools und Dokumentation zu XEN Virtual Machine Monitor
  * XEN-angepasster Linux-Kernel für Hostserver
  * YaST-Module zur Installation und Konfiguration von virtuellen Systemen
-Des.de:
+Des:
Set up a server to configure, manage, and monitor virtual machines on a single physical machine. Includes:
 * XEN virtual machine monitor
 * XEN virtual machine monitor tools and documentation
 * XEN-adapted Linux kernel for host server
 * YaST modules to install and configure virtual systems
-Des:
+Des.es:
Instala un servidor para configurar, gestionar y monitorizar máquinas virtuales en un sólo equipo físico. Incluye:
  * Monitor de máquina virtual XEN
  * Herramientas y documentos de monitorización de máquina virtual XEN
  * Núcleo adaptado a XEN para el servidor host
  * Módulos de YaST para instalar y configurar sistemas virtuales
-Des.es:
+Des.fi:
Aseta palvelin määrittämään, hallitsemaan ja tarkkailemaan virtuaalikoneita yhdessä fyysisessä koneessa. Sisältää:
 * XEN-virtuaalikoneen tarkkailun
 * XEN-virtuaalikoneiden tarkkailutyökalut ja ohjeistuksen
 * XEN-yhteensopivan Linux-ytimen isäntäpalvelimelle
 * YaST-moduulit virtuaalijärjestelmien asentamiseen ja määrittämiseen
-Des.fi:
+Des.fr:
Permet de configurer, gérer et surveiller des machines virtuelles sur un seul et unique poste physique. Comprend :
 * Surveillance de la machine virtuelle XEN
  * Documentation et outils de surveillance de la machine virtuelle XEN
  * Kernel Linux adapté à XEN pour le serveur hôte
  * Modules YaST pour installer et configurer les systèmes virtuels
-Des.fr:
+Des.hu:
Kiszolgáló beállítása egy fizikai gépen több virtuális gép konfigurálásához, felügyeletéhez és figyeléséhez. Részei:
 * XEN virtuálisgép-figyelő
 * XEN virtuálisgép-figyelő eszközök és dokumentáció
 * XEN-hez igazított Linux-kernel a kiszolgálóhoz
 * YaST-modulok a virtuális rendszerek telepítéséhez és beállításához
-Des.hu:
+Des.it:
Impostare un server per la configurazione, la gestione e il monitoraggio dei computer virtuali su un solo computer fisico. Include:
  * Applicazione di monitoraggio dei computer virtuali XEN
  * Strumenti e documentazione dell'applicazione di monitoraggio dei computer virtuali XEN
  * Kernel Linux XEN per il server host
  * Modulo YaST per l'installazione e la configurazione dei sistemi virtuali
-Des.it:
+Des.ja:
1台のコンピュータ上で仮想マシンの設定、管理、モニタができるようサーバを設定します。次を含む:
*XENバーチャルマシンモニタ
*XENバーチャルマシンモニタツールと文書
*ホストサーバ用XEN搭載Linuxカーネル
*仮想システムのインストールと設定用YaSTモジュール
-Des.ja:
+Des.km:
រៀបចំ​ម៉ាស៊ីន​បម្រើ​មួយ​ដើម្បី​កំណត់​រចនាសម្ព័ន្ធ គ្រប់គ្រង និង ត្រួតពិនិត្យ​ម៉ាស៊ីន​និម្មិត​នៅ​លើ​ម៉ាស៊ីន​ពិត​មួយ ។ ក្នុង​នោះ​រួម​មាន ៖
 * កម្មវិធី​ត្រួតពិនិត្យ​ម៉ាស៊ីន​និម្មិត XEN
 * ឧបករណ៍ និង ឯកសារ​សម្រាប់​ត្រួតពិនិត្យ​ម៉ាស៊ីន​និម្មិត XEN
 * ខឺណែល​លីនុច​ដែល​បាន​សម្រួល​តាម XEN សម្រាប់​ម៉ាស៊ីន​បម្រើ​ម៉ាស៊ីន​ភ្ញៀវ
 * ម៉ូឌុល YaST ដើម្បី​ដំឡើង និង កំណត់​រចនាសម្ព័ន្ធ​ប្រព័ន្ធ​និម្មិត
-Des.km:
+Des.nb:
Sett opp en server for å konfigurere, administrere og overvåke virtuelle maskiner på en enkelt fysisk maskin. Omfatter:
 * Overvåking av virtuell XEN-maskin
 * Overvåkingsverktøy og dokumentasjon for virtuell XEN-maskin
 * XEN-tilpasset Linux-kjerne for vertsmaskinserver
 * YaST-moduler for installasjon og konfigurasjon av virtuelle systemer
-Des.nb:
+Des.nl:
Stel een server in voor het configureren, beheren en monitoren van virtuele machines op één fysieke computer. Inclusief:
 * XEN virtual machine-monitor
 * XEN virtual machine-monitorgereedschappen en documentatie
 * XEN-adapted Linux-kernel voor hostserver
 * YaST-modules voor het installeren en configureren van virtuele systemen
-Des.nl:
+Des.pl:
Serwer pozwalający na konfigurację, zarządzanie i nadzorowanie komputerówwirtualnych na jednym komputerze fizycznym. Zawiera:
 * monitor komputerów wirtualnych XEN
 * narzędzia monitorujące komputer wirtualny XEN wraz z dokumentacją
 * jądro Linux zaadaptowane dla XEN dla serwera hostów
 * moduły dla programu YaST pozwalające na instalację i konfigurację systemów wirtualnych
-Des.pl:
+Des.pt:
Configura um servidor para criar, gerir e monitorizar máquinas virtuais numa única máquina física. Inclui:
 * Monitor XEN de máquina virtual
 * Documentação e ferramentas de monitor XEN de máquina virtual
 * Kernel de Linux adaptado a XEN para o servidor anfitrião
 * Módulos YaST para instalar e configurar sistemas virtuais
-Des.pt:
+Des.pt_BR:
Configura um servidor para configurar, gerenciar e monitorar computadores virtuais em um único computador físico. Inclui:
  * Monitor de máquina virtual do XEN
  * Ferramentas e documentação do monitor de máquina virtual do XEN
  * Kernel do Linux adaptado para o XEN para servidores host
  * Módulos do YaST para instalação e configuração de sistemas virtuais
-Des.pt_BR:
+Des.zh_CN:
设置服务器来配置、管理和监视单个物理机器上的虚拟机。包括：
  * XEN 虚拟机监视
  * XEN 虚拟机监视工具和文档
  * 适合 XEN 的 Linux 内核（用于主机服务器）
  * YaST 模块，用于安装和配置虚拟系统
-Des.zh_CN:
+Des.zh_TW:
設定伺服器以在單一實體機器上設定、管理及監控虛擬機器。包括：
  * XEN 虛擬機器監控
  * XEN 虛擬機器監控工具與文件
  * 針對主機伺服器 XEN 調整過的 Linux 核心
  * 安裝及設定虛擬系統的 YaST 模組
-Des.zh_TW:

+Req:
basesystem
-Req:

=Vis: true

=Ord: 3100

+Prq:
bridge-utils
xen
xen-libs
xen-tools
xen-tools-ioemu
-Prq:
+Prc:
xen-doc-html
xen-doc-pdf
xterm
yast2-vm
-Prc:

