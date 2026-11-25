#include "port_text_i18n.h"

#include <string.h>

/* One row per English UI string, columns in PortTextLanguage order:
 * EN, FR, DE, ES, IT. The source is UTF-8; the renderer maps it to the
 * game's charmap (see Port_Text_Utf8Next). Text is kept short: the
 * bottom-screen font is wide and chips are sized to their words. */
static const char* const kEntries[][PORT_TEXT_COUNT] = {
    { "AUTO", "AUTO", "AUTO", "AUTO", "AUTO", "自动" },
    { "BILINEAR", "BILINÉAIRE", "BILINEAR", "BILINEAL", "BILINEARE", "双线性" },
    { "PIXEL P.", "PIXEL P.", "PIXELG.", "PÍXEL EX.", "PIXEL P.", "像素精确" },
    { "LOW", "BAS", "NIEDRIG", "BAJO", "BASSO", "低" },
    { "MED", "MOY", "MITTEL", "MEDIO", "MEDIO", "中" },
    { "HIGH", "HAUT", "HOCH", "ALTO", "ALTO", "高" },
    { "FRAME", "CADRE", "RAHMEN", "MARCO", "CORNICE", "边框" },
    { "GBA GRID", "GRILLE GBA", "GBA-RASTER", "REJILLA GBA", "GRIGLIA GBA", "GBA 网格" },
    { "TURBO", "TURBO", "TURBO", "TURBO", "TURBO", "加速" },
    { "GAME", "JEU", "SPIEL", "JUEGO", "GIOCO", "游戏" },
    { "INTERFACE", "INTERFACE", "OBERFLÄCHE", "INTERFAZ", "INTERFACCIA", "界面" },
    { "SYSTEM", "SYSTÈME", "SYSTEM", "SISTEMA", "SISTEMA", "系统" },
    { "LAYERS", "COUCHES", "EBENEN", "CAPAS", "LIVELLI", "图层" },
    { "DEPTH", "PROFONDEUR", "TIEFE", "PROFUNDIDAD", "PROFONDITÀ", "景深" },
    { "THE 3D SLIDER CAN GIVE DEPTH TO THE", "LE CURSEUR 3D PEUT DONNER DU RELIEF", "DER 3D-REGLER KANN DER WELT BEIM SPIELEN",
      "EL REGULADOR 3D PUEDE DAR PROFUNDIDAD", "IL CURSORE 3D PUÒ DARE PROFONDITÀ AL", "3D 滑杆可在游玩时" },
    { "WORLD WHILE YOU PLAY. THE GAME WAS", "AU MONDE PENDANT LE JEU. LE JEU A ÉTÉ", "TIEFE GEBEN. DAS SPIEL WURDE FÜR FLACHES",
      "AL MUNDO MIENTRAS JUEGAS. EL JUEGO SE", "MONDO MENTRE GIOCHI. IL GIOCO È FATTO", "为世界增加景深。本作" },
    { "DRAWN FOR FLAT 2D, SO SOME SCENES MAY", "DESSINÉ EN 2D, DONC CERTAINES SCÈNES", "2D GEZEICHNET, DARUM KÖNNEN EINIGE",
      "DIBUJÓ EN 2D, ASÍ QUE ALGUNAS ESCENAS", "IN 2D, QUINDI ALCUNE SCENE POSSONO", "原为平面 2D 绘制，部分" },
    { "SHOW GRAPHIC ERRORS THAT ARE HARD TO", "PEUVENT AVOIR DES ERREURS GRAPHIQUES", "SZENEN GRAFIKFEHLER ZEIGEN, DIE SCHWER",
      "PUEDEN TENER ERRORES GRÁFICOS DIFÍCILES", "AVERE ERRORI GRAFICI DIFFICILI DA", "场景可能出现难以修复的" },
    { "FIX. THE REST LOOKS GREAT.", "DIFFICILES À CORRIGER. LE RESTE EST BEAU.", "ZU BEHEBEN SIND. DER REST SIEHT TOLL AUS.",
      "DE ARREGLAR. LO DEMÁS QUEDA MUY BIEN.", "CORREGGERE. IL RESTO È MOLTO BELLO.", "画面错误。其余效果很好。" },
    { "TURN IT ON ANYWAY?", "L'ACTIVER QUAND MÊME ?", "TROTZDEM EINSCHALTEN?", "¿ACTIVARLO IGUALMENTE?", "ATTIVARLO COMUNQUE?", "仍要开启吗？" },
    { "(YOU CAN CHANGE IT IN OPTIONS)", "(MODIFIABLE DANS OPTIONS)", "(ÄNDERBAR UNTER OPTIONEN)", "(SE CAMBIA EN OPCIONES)", "(MODIFICABILE IN OPZIONI)", "（可在选项中更改）" },
    { "QUEST", "QUÊTE", "QUEST", "MISIÓN", "MISSIONE", "任务" },
    { "MAP", "CARTE", "KARTE", "MAPA", "MAPPA", "地图" },
    { "ITEMS", "OBJETS", "ITEMS", "OBJETOS", "OGGETTI", "道具" },
    { "BACK", "RETOUR", "ZURÜCK", "ATRÁS", "INDIETRO", "返回" },
    { "CANCEL", "ANNULER", "ABBRECHEN", "CANCELAR", "ANNULLA", "取消" },
    { "LOAD", "CHARGER", "LADEN", "CARGAR", "CARICA", "载入" },
    { "TOP HUD", "HUD HAUT", "HUD OBEN", "HUD ARRIBA", "HUD ALTO", "顶部状态" },
    { "WIDESCREEN", "PANORAMIQUE", "BREITBILD", "PANTALLA ANCHA", "SCHERMO LARGO", "宽屏" },
    { "FOLLOW CAM", "CAMÉRA SUIVI", "KAMERA FOLGT", "CÁMARA SIGUE", "CAMERA SEGUE", "镜头跟随" },
    { "WINDCREST PINS", "REPÈRES VENT", "WINDMARKEN", "MARCAS DE VIENTO", "SEGNALI VENTO", "风纹路标" },
    { "FLOOR AUTO RETURN", "RETOUR ÉTAGE AUTO", "ETAGE AUTO ZURÜCK", "VOLVER PISO AUTO", "PIANO AUTO INDIETRO", "自动回到楼层" },
    { "TURBO SPEED", "VITESSE TURBO", "TURBO-TEMPO", "VELOCIDAD TURBO", "VELOCITÀ TURBO", "加速倍率" },
    { "MASTER VOLUME", "VOLUME GÉNÉRAL", "LAUTSTÄRKE", "VOLUMEN GENERAL", "VOLUME GENERALE", "总音量" },
    { "AUTOSAVE", "SAUV. AUTO", "AUTOSAVE", "AUTOGUARDAR", "SALVA AUTO", "自动存档" },
    { "COLOR CORRECTION", "CORRECTION COULEUR", "FARBKORREKTUR", "CORRECCIÓN COLOR", "CORREZ. COLORE", "色彩校正" },
    { "SHOW FPS", "VOIR FPS", "FPS ZEIGEN", "VER FPS", "MOSTRA FPS", "显示帧率" },
    { "HOLD TO ADVANCE TEXT", "MAINTENIR: TEXTE SUIVANT", "HALTEN: TEXT WEITER", "MANTENER: AVANZAR TEXTO", "TIENI: AVANZA TESTO", "长按推进文本" },
    { "RANDOMIZER", "RANDOMISEUR", "RANDOMIZER", "RANDOMIZADOR", "RANDOMIZER", "随机模式" },
    { "PANEL BACKDROP", "FOND DU PANNEAU", "PANEL-HINTERGRUND", "FONDO DEL PANEL", "SFONDO PANNELLO", "面板背景" },
    { "SWAP SCREENS", "ÉCHANGER ÉCRANS", "SCREENS TAUSCHEN", "INTERCAMBIAR PANTALLAS", "SCAMBIA SCHERMI", "交换屏幕" },
    { "ASPECT RATIO", "FORMAT IMAGE", "SEITENVERHÄLTNIS", "RELACIÓN ASPECTO", "FORMATO IMMAGINE", "画面比例" },
    { "DISPLAY STYLE", "STYLE AFFICHAGE", "ANZEIGESTIL", "ESTILO IMAGEN", "STILE VISUALE", "显示样式" },
    { "FRAMES", "IMAGES", "FRAMES", "FOTOGRAMAS", "FRAME", "帧" },
    { "NEIGHBOURS", "SALLES VOIS", "NACHBARN", "SALA VECINA", "STANZE VIC.", "相邻房间" },
    { "3D WORLD", "MONDE 3D", "3D-WELT", "MUNDO 3D", "MONDO 3D", "3D 世界" },
    { "SHOW", "VOIR", "ZEIGEN", "VER", "MOSTRA", "显示" },
    { "HIDE", "CACHER", "VERBERGEN", "OCULTAR", "NASCONDI", "隐藏" },
    { "ON", "OUI", "AN", "SÍ", "SÌ", "开" },
    { "OFF", "NON", "AUS", "NO", "NO", "关" },
    { "RESTART", "REDÉMARRER", "NEUSTART", "REINICIAR", "RIAVVIA", "重启" },
    { "ORIGINAL", "ORIGINAL", "ORIGINAL", "ORIGINAL", "ORIGINALE", "原始" },
    { "PIXEL PERFECT", "PIXEL PARFAIT", "PIXELGENAU", "PÍXEL EXACTO", "PIXEL PERFETTO", "像素精确" },
    { "BLUR", "FLOU", "WEICH", "SUAVE", "SFUOCATO", "模糊" },
    { "PATTERN", "MOTIF", "MUSTER", "PATRÓN", "MOTIVO", "图案" },
    { "CREAM", "CRÈME", "CREME", "CREMA", "CREMA", "米白" },
    { "DARK", "SOMBRE", "DUNKEL", "OSCURO", "SCURO", "深色" },
    { "DIM", "TAMISÉ", "GEDIMMT", "TENUE", "SOFFUSO", "暗色" },
    { "STONE", "PIERRE", "STEIN", "PIEDRA", "PIETRA", "石灰" },
    { "SLATE", "ARDOISE", "SCHIEFER", "PIZARRA", "ARDESIA", "石板" },
    { "NAVY", "MARINE", "MARINE", "MARINO", "BLU", "藏青" },
    { "LOCK 30", "FIXE 30", "FIX 30", "FIJO 30", "FISSO 30", "锁定 30" },
    { "NO SKIP", "SANS SAUT", "KEIN SKIP", "SIN SALTOS", "NO SALTI", "不跳帧" },
    { "SCREEN", "ÉCRAN", "BILDSCHIRM", "PANTALLA", "SCHERMO", "屏幕" },
    { "GAMEPLAY", "JEU", "SPIEL", "JUEGO", "GIOCO", "玩法" },
    { "DEVELOPER", "DÉVELOPPEUR", "ENTWICKLER", "DESARROLLO", "SVILUPPO", "开发" },
    { "OVERLAY", "SUPERPOSITION", "OVERLAY", "SUPERPOSICIÓN", "OVERLAY", "叠加" },
    { "UPDATE", "MISE À JOUR", "UPDATE", "ACTUALIZAR", "AGGIORNA", "更新" },
    { "SETTINGS", "RÉGLAGES", "OPTIONEN", "AJUSTES", "OPZIONI", "设置" },
    { "VERSION", "VERSION", "VERSION", "VERSIÓN", "VERSIONE", "版本" },
    { "MODEL", "MODÈLE", "MODELL", "MODELO", "MODELLO", "机型" },
    { "FPS NOW", "FPS ACTUEL", "FPS JETZT", "FPS AHORA", "FPS ORA", "当前帧率" },
    { "FPS AVG", "FPS MOYEN", "FPS MITTEL", "FPS MEDIO", "FPS MEDI", "平均帧率" },
    { "AREA", "ZONE", "BEREICH", "ZONA", "AREA", "区域" },
    { "ROOM", "SALLE", "RAUM", "SALA", "STANZA", "房间" },
    { "WIDE", "LARGE", "BREIT", "ANCHO", "LARGO", "宽屏" },
    { "ONLY IN VIEW: ORIGINAL", "SEULEMENT VUE: ORIGINAL", "NUR ANSICHT: ORIGINAL", "SOLO EN VISTA: ORIGINAL", "SOLO VISTA: ORIGINALE", "仅视野内:原版" },
    { "WIDE + AI", "LARGE + IA", "BREIT + KI", "ANCHO + IA", "LARGO + IA", "宽屏 + AI" },
    { "WIDE VIEW", "VUE LARGE", "BREITBILD", "VISTA ANCHA", "VISTA LARGA", "宽屏视野" },
    { "SOME SCENES HAVE NOTHING PAST THE", "CERTAINES SCÈNES N'ONT RIEN AU-DELÀ DES", "EINIGE SZENEN HABEN NICHTS JENSEITS DER",
      "ALGUNAS ESCENAS NO TIENEN NADA MÁS ALLÁ", "ALCUNE SCENE NON HANNO NULLA OLTRE I", "部分场景在 GBA 画面" },
    { "EDGES OF THE GBA SCREEN. NEW EDGES", "BORDS DE L'ÉCRAN GBA. DE NOUVEAUX BORDS", "GBA-BILDRÄNDER. DAFÜR WURDEN NEUE RÄNDER",
      "DE LOS BORDES DE LA PANTALLA GBA. PARA", "BORDI DELLO SCHERMO GBA. PER LORO SONO", "边界之外空无一物。这些" },
    { "WERE MADE FOR THEM WITH A GENERATIVE", "ONT ÉTÉ CRÉÉS PAR UNE IA GÉNÉRATIVE", "MIT EINER GENERATIVEN KI ERSTELLT,",
      "ELLAS SE CREARON BORDES NUEVOS CON UNA", "STATI CREATI NUOVI BORDI CON UN'IA", "边界已用生成式 AI 补画" },
    { "AI TO FILL THE WIDE SCREEN.", "POUR REMPLIR L'ÉCRAN LARGE.", "DAMIT SIE DEN BREITEN BILDSCHIRM FÜLLEN.",
      "IA GENERATIVA PARA LLENAR LA PANTALLA.", "GENERATIVA PER RIEMPIRE LO SCHERMO.", "以填满宽屏。" },
    { "SHOW THESE EDGES?", "AFFICHER CES BORDS ?", "DIESE RÄNDER ANZEIGEN?", "¿VER ESOS BORDES?", "MOSTRARE QUESTI BORDI?", "显示这些边界吗？" },
    { "(YOU CAN CHANGE IT IN VIEW)", "(MODIFIABLE DANS VUE)", "(ÄNDERBAR UNTER ANSICHT)", "(SE CAMBIA EN VISTA)", "(MODIFICABILE IN VISTA)", "（可在视野中更改）" },
    { "YES", "OUI", "JA", "SÍ", "SÌ", "是" },
    { "NO", "NON", "NEIN", "NO", "NO", "否" },
    { "NATIVE", "NATIF", "NATIV", "NATIVO", "NATIVO", "原生" },
    { "LOAD LATEST DUMP?", "CHARGER LA DERNIÈRE COPIE ?", "NEUESTEN DUMP LADEN?", "¿CARGAR ÚLTIMO VOLCADO?", "CARICARE L'ULTIMO DUMP?", "载入最新转储？" },
    { "THE LATEST DUMP IN THE", "LA DERNIÈRE COPIE DU", "DER NEUESTE DUMP IM", "EL ÚLTIMO VOLCADO DE LA", "L'ULTIMO DUMP NELLA", "dumps 文件夹中的" },
    { "DUMPS FOLDER WILL REPLACE", "DOSSIER DUMPS REMPLACERA", "DUMPS-ORDNER ERSETZT", "CARPETA DUMPS REEMPLAZARÁ", "CARTELLA DUMPS SOSTITUIRÀ", "最新转储将替换" },
    { "THE CURRENT GAME STATE.", "L'ÉTAT ACTUEL DU JEU.", "DEN AKTUELLEN SPIELSTAND.", "EL ESTADO ACTUAL DEL JUEGO.", "LO STATO ATTUALE DEL GIOCO.", "当前游戏状态。" },
    { "UNSAVED PROGRESS MAY BE LOST.", "PROGRESSION NON SAUVÉE PERDUE.", "UNGESPEICHERTES GEHT VERLOREN.",
      "SE PIERDE LO NO GUARDADO.", "PROGRESSI NON SALVATI PERSI.", "未保存的进度可能丢失。" },
    { "THE GAME WILL RESTART.", "LE JEU VA REDÉMARRER.", "DAS SPIEL STARTET NEU.", "EL JUEGO SE REINICIARÁ.", "IL GIOCO VERRÀ RIAVVIATO.", "游戏将重启。" },
    { "THIS OPTION IS NOT", "CETTE OPTION N'EST PAS", "DIESE OPTION IST", "ESTA OPCIÓN AÚN NO", "QUESTA OPZIONE NON È", "该选项" },
    { "AVAILABLE YET.", "ENCORE DISPONIBLE.", "NOCH NICHT VERFÜGBAR.", "ESTÁ DISPONIBLE.", "ANCORA DISPONIBILE.", "尚不可用。" },
    { "DROP", "POSER", "ABLEGEN", "SOLTAR", "LASCIA", "放下" },
    { "THROW", "LANCER", "WERFEN", "LANZAR", "LANCIA", "投掷" },
    { "READ", "LIRE", "LESEN", "LEER", "LEGGI", "阅读" },
    { "CHECK", "EXAMINER", "ANSEHEN", "MIRAR", "ESAMINA", "查看" },
    { "OPEN", "OUVRIR", "ÖFFNEN", "ABRIR", "APRI", "打开" },
    { "SPEAK", "PARLER", "SPRECHEN", "HABLAR", "PARLA", "交谈" },
    { "GRAB", "PRENDRE", "GREIFEN", "AGARRAR", "AFFERRA", "抓住" },
    { "LIFT", "SOULEVER", "HEBEN", "LEVANTAR", "SOLLEVA", "举起" },
    { "GROW", "GRANDIR", "WACHSEN", "CRECER", "INGRANDISCI", "变大" },
    { "SHRINK", "RÉTRÉCIR", "SCHRUMPFEN", "ENCOGER", "RIMPICCIOLISCI", "缩小" },
    { "ROLL", "ROULER", "ROLLEN", "RODAR", "ROTOLA", "卷动" },
    { "Deepwood Shrine", "Temple de la Forêt", "Wald-Schrein", "Sepulcro del bosque", "Tempietto della Foresta", "森之神殿" },
    { "Cave of Flames", "Grotte de Feu", "Höhle der Flamme", "Cueva de las llamas", "Grotta del Fuoco", "火焰洞窟" },
    { "Fortress of Winds", "Forteresse du Vent", "Tor des Windes", "Arco de los vientos", "Bastioni di Maestrale", "风之要塞" },
    { "Temple of Droplets", "Temple de l'Eau", "Tempel des Tropfens", "Templo de las aguas", "Tempio delle Gocce", "水滴神殿" },
    { "Palace of Winds", "Palais du Vent", "Palast des Windes", "Palacio de los vientos", "Palazzo dell'Aria", "风之神殿" },
    { "Dark Hyrule Castle", "Château des Ténèbres", "Dunkles Schloss Hyrule", "Castillo de Hyrule tenebroso", "Castello di Hyrule nelle tenebre", "暗黑海拉尔城" },
    { "THE GAME SLOWS DOWN IF IT CANNOT KEEP UP", "LE JEU RALENTIT S'IL NE SUIT PAS", "DAS SPIEL WIRD LANGSAMER, WENN ES NICHT MITKOMMT",
      "EL JUEGO VA MÁS LENTO SI NO PUEDE MÁS", "IL GIOCO RALLENTA SE NON REGGE", "性能不足时游戏会变慢" },
    { "30 FPS SHOWN, GAME AT FULL SPEED", "30 FPS AFFICHÉS, JEU À VITESSE NORMALE", "30 FPS ANGEZEIGT, SPIEL VOLLE GESCHWINDIGKEIT",
      "30 FPS MOSTRADOS, JUEGO A VELOCIDAD NORMAL", "30 FPS MOSTRATI, GIOCO A VELOCITÀ NORMALE", "显示 30 帧，游戏全速" },
    { "CHANNEL", "CANAL", "KANAL", "CANAL", "CANALE", "通道" },
    { "STABLE", "STABLE", "STABIL", "ESTABLE", "STABILE", "稳定版" },
    { "CHECK FOR UPDATE", "CHERCHER UNE MISE À JOUR", "NACH UPDATE SUCHEN", "BUSCAR ACTUALIZACIÓN", "CERCA AGGIORNAMENTI", "检查更新" },
    { "UNSAVED PROGRESS IS LOST", "PROGRESSION NON SAUVÉE PERDUE", "UNGESPEICHERTES GEHT VERLOREN",
      "SE PIERDE LO NO GUARDADO", "PROGRESSI NON SALVATI PERSI", "未保存的进度将丢失" },
    { "PREV", "PRÉC.", "ZURÜCK", "ANT.", "PREC.", "上一页" },
    { "NEXT", "SUIV.", "WEITER", "SIG.", "SUCC.", "下一页" },
    { "INSTALL UPDATE", "INSTALLER", "INSTALLIEREN", "INSTALAR", "INSTALLA", "安装更新" },
    { "PLEASE WAIT", "PATIENTEZ", "BITTE WARTEN", "ESPERA", "ATTENDI", "请稍候" },
    { "DOWNLOAD UPDATE", "TÉLÉCHARGER", "HERUNTERLADEN", "DESCARGAR", "SCARICA", "下载更新" },
    { "UPDATE INSTALLED", "MISE À JOUR INSTALLÉE", "UPDATE INSTALLIERT", "ACTUALIZACIÓN INSTALADA", "AGGIORNAMENTO INSTALLATO", "更新已安装" },
    { "INSTALLED", "INSTALLÉE", "INSTALLIERT", "INSTALADA", "INSTALLATA", "已安装" },
    { "SD CARD WRITE FAILED", "ÉCRITURE CARTE SD ÉCHOUÉE", "SD-SCHREIBFEHLER", "ERROR ESCRIBIENDO EN SD", "SCRITTURA SD FALLITA", "SD 卡写入失败" },
    { "NO WI-FI CONNECTION", "PAS DE CONNEXION WI-FI", "KEINE WLAN-VERBINDUNG", "SIN CONEXIÓN WI-FI", "NESSUNA CONNESSIONE WI-FI", "无 Wi-Fi 连接" },
    { "TLS SERVICE FAILED", "SERVICE TLS ÉCHOUÉ", "TLS-DIENST FEHLGESCHLAGEN", "FALLÓ EL SERVICIO TLS", "SERVIZIO TLS FALLITO", "TLS 服务失败" },
    { "INVALID RELEASE DATA", "DONNÉES DE VERSION INVALIDES", "UNGÜLTIGE VERSIONSDATEN", "DATOS DE VERSIÓN NO VÁLIDOS", "DATI VERSIONE NON VALIDI", "版本数据无效" },
    { "NO PRE-RELEASE AVAILABLE", "AUCUNE BÊTA DISPONIBLE", "KEINE BETA VERFÜGBAR", "NO HAY BETA DISPONIBLE", "NESSUNA BETA DISPONIBILE", "无可用测试版" },
    { "AUTO UPDATE", "MAJ AUTO", "AUTO-UPDATE", "AUTOACTUALIZAR", "AGG. AUTO", "自动更新" },
    { "NEW VERSION:", "NOUVELLE VERSION :", "NEUE VERSION:", "NUEVA VERSIÓN:", "NUOVA VERSIONE:", "新版本:" },
    { "INSTALL IT NOW?", "L'INSTALLER MAINTENANT ?", "JETZT INSTALLIEREN?", "¿INSTALARLA AHORA?", "INSTALLARLA ORA?", "现在安装吗？" },
    { "(OPTIONS -> UPDATES: AUTO UPDATE)", "(OPTIONS -> MISE À JOUR : MAJ AUTO)", "(OPTIONEN -> UPDATES: AUTO-UPDATE)",
      "(OPCIONES -> ACTUALIZAR: AUTOACTUALIZAR)", "(OPZIONI -> AGGIORNA: AGG. AUTO)", "（选项->更新:自动更新）" },
    { "WHAT'S NEW", "NOUVEAUTÉS", "NEUERUNGEN", "NOVEDADES", "NOVITÀ", "更新内容" },
    { "LATER", "PLUS TARD", "SPÄTER", "LUEGO", "DOPO", "稍后" },
    { "INSTALL", "INSTALLER", "INSTALLIEREN", "INSTALAR", "INSTALLA", "安装" },
    { "UPDATING", "MISE À JOUR", "UPDATE LÄUFT", "ACTUALIZANDO", "AGGIORNAMENTO", "更新中" },
    { "DO NOT TURN THE CONSOLE OFF.", "N'ÉTEIGNEZ PAS LA CONSOLE.", "KONSOLE NICHT AUSSCHALTEN.", "NO APAGUES LA CONSOLA.", "NON SPEGNERE LA CONSOLE.", "请勿关闭主机。" },
    { "UPDATE FAILED", "ÉCHEC DE LA MISE À JOUR", "UPDATE FEHLGESCHLAGEN", "FALLÓ LA ACTUALIZACIÓN", "AGGIORNAMENTO FALLITO", "更新失败" },
    { "THE GAME KEEPS RUNNING AS IT WAS.", "LE JEU RESTE INCHANGÉ.", "DAS SPIEL BLEIBT UNVERÄNDERT.",
      "EL JUEGO SIGUE COMO ESTABA.", "IL GIOCO RESTA COME ERA.", "游戏将保持原样继续。" },
    { "OK", "OK", "OK", "OK", "OK", "确定" },
    { "CLOSE", "FERMER", "SCHLIESSEN", "CERRAR", "CHIUDI", "关闭" },
    { "NO RELEASE AVAILABLE", "AUCUNE VERSION DISPONIBLE", "KEINE VERSION VERFÜGBAR", "NO HAY VERSIÓN DISPONIBLE", "NESSUNA VERSIONE DISPONIBILE", "无可用版本" },
    { "NO STABLE RELEASE AVAILABLE", "AUCUNE VERSION STABLE DISPONIBLE", "KEINE STABILE VERSION VERFÜGBAR",
      "NO HAY VERSIÓN ESTABLE DISPONIBLE", "NESSUNA VERSIONE STABILE DISPONIBILE", "无可用稳定版" },
    { "RESTARTING THE GAME", "REDÉMARRAGE DU JEU", "SPIEL STARTET NEU", "REINICIANDO EL JUEGO", "RIAVVIO DEL GIOCO", "正在重启游戏" },
    { "UPDATE AVAILABLE", "MISE À JOUR DISPONIBLE", "UPDATE VERFÜGBAR", "ACTUALIZACIÓN DISPONIBLE", "AGGIORNAMENTO DISPONIBILE", "有可用更新" },
    { "YOU ARE UP TO DATE", "VOUS ÊTES À JOUR", "SIE SIND AKTUELL", "ESTÁS AL DÍA", "SEI AGGIORNATO", "已是最新版本" },
    { "NOT ENOUGH SD SPACE", "ESPACE SD INSUFFISANT", "NICHT GENUG SD-SPEICHER", "SD SIN ESPACIO SUFICIENTE", "SPAZIO SD INSUFFICIENTE", "SD 卡空间不足" },
    { "3DSX LAUNCH PATH UNKNOWN", "CHEMIN 3DSX INCONNU", "3DSX-PFAD UNBEKANNT", "RUTA 3DSX DESCONOCIDA", "PERCORSO 3DSX IGNOTO", "未知 3DSX 启动路径" },
    { "CANNOT WRITE TO SD CARD", "ÉCRITURE SD IMPOSSIBLE", "SD NICHT BESCHREIBBAR", "NO SE PUEDE ESCRIBIR EN SD", "IMPOSSIBILE SCRIVERE SU SD", "无法写入 SD 卡" },
    { "VERIFYING DOWNLOAD", "VÉRIFICATION", "DOWNLOAD WIRD GEPRÜFT", "VERIFICANDO DESCARGA", "VERIFICA DOWNLOAD", "正在校验下载" },
    { "DOWNLOAD CHECK FAILED", "VÉRIFICATION ÉCHOUÉE", "PRÜFUNG FEHLGESCHLAGEN", "FALLÓ LA VERIFICACIÓN", "VERIFICA FALLITA", "下载校验失败" },
    { "INSTALLING UPDATE", "INSTALLATION EN COURS", "UPDATE WIRD INSTALLIERT", "INSTALANDO ACTUALIZACIÓN", "INSTALLAZIONE IN CORSO", "正在安装更新" },
    { "INSTALLATION FAILED", "INSTALLATION ÉCHOUÉE", "INSTALLATION FEHLGESCHLAGEN", "FALLÓ LA INSTALACIÓN", "INSTALLAZIONE FALLITA", "安装失败" },
    { "UPDATE CANCELLED", "MISE À JOUR ANNULÉE", "UPDATE ABGEBROCHEN", "ACTUALIZACIÓN CANCELADA", "AGGIORNAMENTO ANNULLATO", "更新已取消" },
    { "CONNECTION FAILED - RETRY", "CONNEXION ÉCHOUÉE - RÉESSAYER", "VERBINDUNG FEHLGESCHLAGEN - NOCHMAL",
      "CONEXIÓN FALLIDA - REINTENTA", "CONNESSIONE FALLITA - RIPROVA", "连接失败 - 重试" },
    { "DOWNLOADING UPDATE", "TÉLÉCHARGEMENT", "UPDATE WIRD GELADEN", "DESCARGANDO ACTUALIZACIÓN", "DOWNLOAD IN CORSO", "正在下载更新" },
    { "CHECKING FOR UPDATES", "RECHERCHE EN COURS", "SUCHE NACH UPDATES", "BUSCANDO ACTUALIZACIONES", "RICERCA AGGIORNAMENTI", "正在检查更新" },
    { "CANNOT START UPDATE", "IMPOSSIBLE DE LANCER", "UPDATE NICHT STARTBAR", "NO SE PUEDE INICIAR", "IMPOSSIBILE AVVIARE", "无法启动更新" },
    { "CANNOT SAVE UPDATE CHANNEL", "CANAL NON SAUVEGARDÉ", "KANAL NICHT GESPEICHERT", "NO SE GUARDÓ EL CANAL", "CANALE NON SALVATO", "无法保存更新通道" },
    { "VIEW", "VUE", "ANSICHT", "VISTA", "VISTA", "视野" },
    { "STYLE", "STYLE", "STIL", "ESTILO", "STILE", "样式" },
    { "VOLUME", "VOLUME", "LAUTSTÄRKE", "VOLUMEN", "VOLUME", "音量" },
    { "HOLD TEXT", "TEXTE VITE", "TEXT HALTEN", "TEXTO VELOZ", "TIENI TESTO", "长按文本" },
    { "MAP FOLLOW", "SUIVI CARTE", "KARTE FOLGT", "SEGUIR MAPA", "SEGUI MAPPA", "地图跟随" },
    { "CREST PINS", "REPÈRES", "MARKEN", "MARCAS", "SEGNALI", "纹章标记" },
    { "FLOOR BACK", "RETOUR ÉT.", "ETAGE ZUR.", "VOLVER PISO", "TORNA PIANO", "楼层返回" },
    { "COLOURS", "COULEURS", "FARBEN", "COLORES", "COLORI", "配色" },
    { "THEME", "THÈME", "DESIGN", "TEMA", "TEMA", "主题" },
    { "TABS", "ONGLETS", "REITER", "PESTAÑAS", "SCHEDE", "标签" },
    { "TOP", "HAUT", "OBEN", "ARRIBA", "ALTO", "顶部" },
    { "BOTTOM", "BAS", "UNTEN", "ABAJO", "BASSO", "底部" },
    { "UPDATES", "MISES À JOUR", "UPDATES", "ACTUALIZACIONES", "AGGIORNAMENTI", "更新" },
    { "NEW VERSION", "NOUVELLE VERSION", "NEUE VERSION", "NUEVA VERSIÓN", "NUOVA VERSIONE", "新版本" },
    { "+ BETAS", "+ BÊTAS", "+ BETAS", "+ BETAS", "+ BETA", "+ 测试版" },
    { "PARCHMENT", "PARCHEMIN", "PERGAMENT", "PERGAMINO", "PERGAMENA", "羊皮纸" },
    { "NIGHT GOLD", "OR NOCTURNE", "NACHTGOLD", "ORO NOCHE", "ORO NOTTE", "夜金" },
    { "FOREST", "FORÊT", "WALD", "BOSQUE", "FORESTA", "森林" },
    { "NONE", "AUCUNE", "KEINE", "NINGUNA", "NESSUNA", "无" },
    { "RELEASE", "VERSION", "VERSION", "VERSIÓN", "VERSIONE", "版本" },
    { "STATUS", "ÉTAT", "STATUS", "ESTADO", "STATO", "状态" },
    { "+ NEW STATE", "+ NOUVEAU", "+ NEU", "+ NUEVO", "+ NUOVO", "+ 新建" },
    { "STATE", "ÉTAT", "STAND", "ESTADO", "STATO", "存档" },
    { "STATES", "ÉTATS", "STÄNDE", "ESTADOS", "STATI", "存档" },
    { "NO SAVE STATES YET", "AUCUN ÉTAT SAUVEGARDÉ", "NOCH KEINE SPIELSTÄNDE", "AÚN NO HAY ESTADOS", "NESSUNO STATO SALVATO", "尚无快速存档" },
    { "+ NEW STATE SAVES ONE", "+ NOUVEAU POUR EN SAUVER UN", "+ NEU SPEICHERT EINEN", "+ NUEVO GUARDA UNO", "+ NUOVO NE SALVA UNO", "+ 新建并保存一个" },
    { "LOAD A GAME TO SAVE ONE", "CHARGEZ UNE PARTIE", "SPIEL LADEN ZUM SPEICHERN", "CARGA UNA PARTIDA", "CARICA UNA PARTITA", "载入游戏后才能保存" },
    { "NO PICTURE", "SANS IMAGE", "KEIN BILD", "SIN IMAGEN", "NESSUN'IMMAGINE", "无画面" },
    { "ANOTHER BUILD:", "AUTRE VERSION :", "ANDERER BUILD:", "OTRA VERSIÓN:", "ALTRA VERSIONE:", "其他版本:" },
    { "CANNOT LOAD", "CHARGEMENT IMPOSSIBLE", "LADEN NICHT MÖGLICH", "NO SE PUEDE CARGAR", "IMPOSSIBILE CARICARE", "无法载入" },
    { "SAVE OVER", "ÉCRASER", "ÜBERSCHR.", "SOBREESCR.", "SOVRASCRIVI", "覆盖保存" },
    { "DELETE", "SUPPRIMER", "LÖSCHEN", "BORRAR", "ELIMINA", "删除" },
    { "SURE?", "SÛR ?", "SICHER?", "¿SEGURO?", "SICURO?", "确定？" },
    { "TAP TWICE TO CONFIRM", "TOUCHER 2 FOIS POUR CONFIRMER", "ZWEIMAL TIPPEN ZUM BESTÄTIGEN",
      "TOCA DOS VECES PARA CONFIRMAR", "TOCCA DUE VOLTE PER CONFERMARE", "连点两次确认" },
    { "LOAD A GAME FIRST", "CHARGEZ D'ABORD UNE PARTIE", "ZUERST SPIEL LADEN", "CARGA PRIMERO UNA PARTIDA", "CARICA PRIMA UNA PARTITA", "请先载入游戏" },
    { "SAVING STATE", "SAUVEGARDE...", "SPEICHERE...", "GUARDANDO...", "SALVATAGGIO...", "正在保存" },
    { "LOADING STATE", "CHARGEMENT...", "LADE...", "CARGANDO...", "CARICAMENTO...", "正在载入" },
    { "DELETING STATE", "SUPPRESSION...", "LÖSCHE...", "BORRANDO...", "ELIMINAZIONE...", "正在删除" },
    { "AVAILABLE ONCE A GAME IS LOADED", "DISPONIBLE QUAND UNE PARTIE EST CHARGÉE", "VERFÜGBAR, SOBALD EIN SPIEL GELADEN IST",
      "DISPONIBLE AL CARGAR UNA PARTIDA", "DISPONIBILE A PARTITA CARICATA", "载入游戏后可用" },
    { "OPTIONS AND DEBUG WORK NOW", "OPTIONS ET DEBUG ACTIFS", "OPTIONEN UND DEBUG GEHEN", "OPCIONES Y DEBUG FUNCIONAN", "OPZIONI E DEBUG ATTIVE", "选项与调试已可用" },
    { "NO MAP YET", "PAS DE CARTE", "NOCH KEINE KARTE", "AÚN SIN MAPA", "NESSUNA MAPPA", "尚无地图" },
    { "FIND THE HYRULE MAP", "TROUVEZ LA CARTE D'HYRULE", "FINDE DIE HYRULE-KARTE", "BUSCA EL MAPA DE HYRULE", "TROVA LA MAPPA DI HYRULE", "寻找海拉尔地图" },
    { "DOWNLOAD AND INSTALL THIS VERSION?", "TÉLÉCHARGER ET INSTALLER ?", "DIESE VERSION LADEN UND INSTALLIEREN?",
      "¿DESCARGAR E INSTALAR ESTA VERSIÓN?", "SCARICARE E INSTALLARE?", "下载并安装此版本？" },
    { "PRESS CHECK TO LOOK FOR A NEW VERSION", "APPUYEZ SUR CHERCHER POUR TROUVER UNE VERSION", "PRÜFEN DRÜCKEN FÜR NEUE VERSION",
      "PULSA BUSCAR PARA NUEVA VERSIÓN", "PREMI CERCA PER NUOVA VERSIONE", "按\"检查\"查找新版本" },
    { "TAP RELEASE FOR THE CHANGELOG", "TOUCHEZ LA VERSION POUR LES NOTES", "VERSION ANTIPPEN FÜR NOTIZEN",
      "TOCA LA VERSIÓN PARA LAS NOTAS", "TOCCA LA VERSIONE PER LE NOTE", "点击版本号看更新日志" },
    { "CHANGELOG", "NOTES", "ÄNDERUNGEN", "NOVEDADES", "NOVITÀ", "更新日志" },
    { "PAGE", "PAGE", "SEITE", "PÁGINA", "PAGINA", "页" },
    { "OF", "SUR", "VON", "DE", "DI", "共" },
    { "MEM DUMP", "COPIE MÉMOIRE", "SPEICHERABBILD", "VOLCADO MEMORIA", "DUMP MEMORIA", "内存转储" },
    { "LOAD STATE", "CHARGER ÉTAT", "STATUS LADEN", "CARGAR ESTADO", "CARICA STATO", "载入存档" },
    { "DONE", "FAIT", "FERTIG", "HECHO", "FATTO", "完成" },
    { "WRITE", "ÉCRIRE", "SCHREIBEN", "ESCRIBIR", "SCRIVI", "写入" },
    { "THE LATEST DUMP IN THE DUMPS FOLDER", "LA DERNIÈRE COPIE DU DOSSIER DUMPS", "DER NEUESTE DUMP IM DUMPS-ORDNER",
      "EL ÚLTIMO VOLCADO DE LA CARPETA DUMPS", "L'ULTIMO DUMP NELLA CARTELLA DUMPS", "dumps 文件夹里的最新转储" },
    { "WILL REPLACE THE CURRENT GAME STATE.", "REMPLACERA L'ÉTAT ACTUEL DU JEU.", "ERSETZT DEN AKTUELLEN SPIELSTAND.",
      "REEMPLAZARÁ EL ESTADO ACTUAL DEL JUEGO.", "SOSTITUIRÀ LO STATO ATTUALE DEL GIOCO.", "将替换当前游戏状态。" },
    { "SAVES FROM THE MINISH CAP 3DS FOUND", "SAUVEGARDES DE THE MINISH CAP 3DS TROUVÉES", "SPIELSTÄNDE VON THE MINISH CAP 3DS GEFUNDEN",
      "HAY PARTIDAS DE THE MINISH CAP 3DS", "TROVATI SALVATAGGI DI THE MINISH CAP 3DS", "发现《缩小帽 3DS》的存档" },
    { "COPY THEM TO THIS APP?", "LES COPIER DANS CETTE APPLI ?", "IN DIESE APP KOPIEREN?", "¿COPIARLAS A ESTA APP?", "COPIARLI IN QUESTA APP?", "复制到本应用？" },
    { "THE ORIGINAL APP KEEPS ITS OWN.", "L'APPLI D'ORIGINE GARDE LES SIENNES.", "DIE ORIGINAL-APP BEHÄLT IHRE.",
      "LA APP ORIGINAL CONSERVA LAS SUYAS.", "L'APP ORIGINALE TIENE I SUOI.", "原应用会保留自己的存档。" },
    { "A: COPY", "A: COPIER", "A: KOPIEREN", "A: COPIAR", "A: COPIA", "A: 复制" },
    { "B: SKIP", "B: IGNORER", "B: ÜBERSPRINGEN", "B: OMITIR", "B: SALTA", "B: 跳过" },
    { "SAVES COPIED", "SAUVEGARDES COPIÉES", "SPIELSTÄNDE KOPIERT", "PARTIDAS COPIADAS", "SALVATAGGI COPIATI", "存档已复制" },
    { "COPY FAILED", "ÉCHEC DE LA COPIE", "KOPIEREN FEHLGESCHLAGEN", "NO SE PUDO COPIAR", "COPIA NON RIUSCITA", "复制失败" },
    { "ERROR", "ERREUR", "FEHLER", "ERROR", "ERRORE", "错误" },
    { "PRESS START TO EXIT.", "START POUR QUITTER.", "START ZUM BEENDEN.", "PULSA START PARA SALIR.", "PREMI START PER USCIRE.", "按 START 退出。" },
    { "STORAGE ERROR", "ERREUR DE STOCKAGE", "SPEICHERFEHLER", "ERROR DE ALMACENAMIENTO", "ERRORE DI ARCHIVIAZIONE", "存储错误" },
    { "COULD NOT OPEN THIS FOLDER:", "IMPOSSIBLE D'OUVRIR CE DOSSIER :", "DIESER ORDNER LÄSST SICH NICHT ÖFFNEN:",
      "NO SE PUDO ABRIR ESTA CARPETA:", "IMPOSSIBILE APRIRE QUESTA CARTELLA:", "无法打开此文件夹:" },
    { "ROM NOT FOUND", "ROM INTROUVABLE", "ROM NICHT GEFUNDEN", "NO SE ENCUENTRA LA ROM", "ROM NON TROVATA", "未找到 ROM" },
    { "UNSUPPORTED ROM", "ROM NON PRISE EN CHARGE", "NICHT UNTERSTÜTZTE ROM", "ROM NO COMPATIBLE", "ROM NON SUPPORTATA", "不支持的 ROM" },
    { "ROM ERROR", "ERREUR DE ROM", "ROM-FEHLER", "ERROR DE ROM", "ERRORE ROM", "ROM 错误" },
    { "COPY YOUR CLEAN USA OR EUROPE ROM TO:", "COPIEZ VOTRE ROM PROPRE USA OU EUROPE DANS :", "KOPIERE DEINE SAUBERE USA- ODER EUROPA-ROM NACH:",
      "COPIA TU ROM LIMPIA DE USA O EUROPA EN:", "COPIA LA TUA ROM PULITA USA O EUROPA IN:", "请把干净的 USA、欧洲或日本版 ROM 放到:" },
    { "THE ORIGINAL PORT'S FOLDER WORKS TOO:", "LE DOSSIER DU PORT D'ORIGINE CONVIENT AUSSI :", "DER ORDNER DES ORIGINAL-PORTS GEHT AUCH:",
      "TAMBIÉN VALE LA CARPETA DEL PORT ORIGINAL:", "VA BENE ANCHE LA CARTELLA DEL PORT ORIGINALE:", "原版移植的文件夹也可以:" },
    { "ANY .GBA FILENAME IS ACCEPTED.", "N'IMPORTE QUEL NOM DE FICHIER .GBA CONVIENT.", "JEDER .GBA-DATEINAME WIRD AKZEPTIERT.",
      "VALE CUALQUIER NOMBRE DE ARCHIVO .GBA.", "QUALSIASI NOME DI FILE .GBA VA BENE.", "任意 .gba 文件名均可。" },
    { "EXPECTED SHA-1:", "SHA-1 ATTENDU :", "ERWARTETER SHA-1:", "SHA-1 ESPERADO:", "SHA-1 PREVISTO:", "期望 SHA-1:" },
    { "EUROPE:", "EUROPE :", "EUROPA:", "EUROPA:", "EUROPA:", "欧洲版:" },
    { "NONE OF THE .GBA FILES IS A SUPPORTED USA (BZME) OR EUROPE (BZMP) ROM.", "AUCUN FICHIER .GBA N'EST UNE ROM USA (BZME) OU EUROPE (BZMP) PRISE EN CHARGE.", "KEINE .GBA-DATEI IST EINE UNTERSTÜTZTE USA- (BZME) ODER EUROPA-ROM (BZMP).",
      "NINGÚN ARCHIVO .GBA ES UNA ROM COMPATIBLE DE USA (BZME) O EUROPA (BZMP).", "NESSUN FILE .GBA È UNA ROM USA (BZME) O EUROPA (BZMP) SUPPORTATA.", "所有 .gba 都不是受支持的 USA (BZME) 或欧洲 (BZMP) ROM。" },
    { "FOLDERS SEARCHED:", "DOSSIERS EXAMINÉS :", "DURCHSUCHTE ORDNER:", "CARPETAS REVISADAS:", "CARTELLE CONTROLLATE:", "已搜索的文件夹:" },
    { "COULD NOT OPEN THE SELECTED .GBA FILE.", "IMPOSSIBLE D'OUVRIR LE FICHIER .GBA CHOISI.", "DIE GEWÄHLTE .GBA-DATEI LÄSST SICH NICHT ÖFFNEN.",
      "NO SE PUDO ABRIR EL ARCHIVO .GBA ELEGIDO.", "IMPOSSIBILE APRIRE IL FILE .GBA SCELTO.", "无法打开所选的 .gba 文件。" },
    { "THIS OPTION IS NOT AVAILABLE YET.", "CETTE OPTION N'EST PAS ENCORE DISPONIBLE.", "DIESE OPTION IST NOCH NICHT VERFÜGBAR.",
      "ESTA OPCIÓN AÚN NO ESTÁ DISPONIBLE.", "QUESTA OPZIONE NON È ANCORA DISPONIBILE.", "该选项尚不可用。" },
};

#define ENTRY_COUNT ((int)(sizeof(kEntries) / sizeof(kEntries[0])))

/* Written by the main thread when the snapshot is built, read by the
 * bottom-screen and updater threads; a single byte, so a stale read only
 * means one frame of the previous language. */
static volatile uint8_t sLanguage = PORT_TEXT_EN;
/* Set once the 3DS build knows the loaded ROM is a Chinese translation. */
static volatile uint8_t sChineseUi = 0;

void Port_Text_SetLanguage(int publicLanguage) {
    /* include/main.h: JP=0, EN=1, FR=2, DE=3, ES=4, IT=5. */
    PortTextLanguage lang = PORT_TEXT_EN;
    if (publicLanguage >= 2 && publicLanguage <= 5) {
        lang = (PortTextLanguage)(publicLanguage - 1);
    }
    /* A Chinese ROM reports JP (0), which would otherwise land on English. */
    if (sChineseUi) {
        lang = PORT_TEXT_ZH;
    }
    sLanguage = (uint8_t)lang;
}

PortTextLanguage Port_Text_GetLanguage(void) {
    return (PortTextLanguage)sLanguage;
}

void Port_Text_SetChineseUi(int enabled) {
    sChineseUi = enabled ? 1u : 0u;
}

int Port_Text_ChineseUi(void) {
    return sChineseUi ? 1 : 0;
}

const char* Port_Text_TrFor(PortTextLanguage lang, const char* english) {
    if (english == NULL || lang <= PORT_TEXT_EN || lang >= PORT_TEXT_COUNT) {
        return english;
    }
    for (int i = 0; i < ENTRY_COUNT; i++) {
        const char* key = kEntries[i][PORT_TEXT_EN];
        if (key[0] == english[0] && strcmp(key, english) == 0) {
            return kEntries[i][lang];
        }
    }
    return english;
}

const char* Port_Text_Tr(const char* english) {
    return Port_Text_TrFor((PortTextLanguage)sLanguage, english);
}

int Port_Text_EntryCount(void) {
    return ENTRY_COUNT;
}

const char* Port_Text_EntryAt(int index, PortTextLanguage lang) {
    if (index < 0 || index >= ENTRY_COUNT || lang < 0 || lang >= PORT_TEXT_COUNT) {
        return NULL;
    }
    return kEntries[index][lang];
}

uint8_t Port_Text_Utf8Next(const char** p) {
    const uint8_t* s = (const uint8_t*)*p;
    uint32_t cp;
    if (s[0] < 0x80) {
        if (s[0] != 0) {
            (*p)++;
        }
        return s[0];
    }
    if ((s[0] & 0xE0) == 0xC0 && (s[1] & 0xC0) == 0x80) {
        cp = ((uint32_t)(s[0] & 0x1F) << 6) | (s[1] & 0x3F);
        *p += 2;
        return cp < 0x100 ? (uint8_t)cp : (uint8_t)'?';
    }
    /* 3/4-byte sequences and stray bytes: skip the lead and its
     * continuation bytes, draw one '?'. */
    (*p)++;
    while ((**(const uint8_t**)p & 0xC0) == 0x80) {
        (*p)++;
    }
    return (uint8_t)'?';
}

char Port_Text_BaseLetter(uint8_t code) {
    static const char kLatin1Base[64] = {
        /* 0xC0 */ 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'C', 'E', 'E', 'E', 'E', 'I', 'I', 'I', 'I',
        /* 0xD0 */ 'D', 'N', 'O', 'O', 'O', 'O', 'O', 0,   'O', 'U', 'U', 'U', 'U', 'Y', 0,   0,
        /* 0xE0 */ 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'c', 'e', 'e', 'e', 'e', 'i', 'i', 'i', 'i',
        /* 0xF0 */ 'd', 'n', 'o', 'o', 'o', 'o', 'o', 0,   'o', 'u', 'u', 'u', 'u', 'y', 0,   'y',
    };
    if (code >= 0xC0) {
        return kLatin1Base[code - 0xC0];
    }
    return 0;
}

uint32_t Port_Text_Utf8CodePoint(const char** p) {
    const uint8_t* s = (const uint8_t*)*p;
    uint32_t cp;
    int extra;
    if (s[0] == 0) {
        return 0;
    }
    if (s[0] < 0x80) {
        (*p)++;
        return s[0];
    }
    if ((s[0] & 0xE0) == 0xC0) {
        cp = s[0] & 0x1Fu;
        extra = 1;
    } else if ((s[0] & 0xF0) == 0xE0) {
        cp = s[0] & 0x0Fu;
        extra = 2;
    } else if ((s[0] & 0xF8) == 0xF0) {
        cp = s[0] & 0x07u;
        extra = 3;
    } else {
        (*p)++;
        return 0xFFFD;
    }
    for (int i = 1; i <= extra; i++) {
        if ((s[i] & 0xC0) != 0x80) {
            (*p)++;
            return 0xFFFD;
        }
        cp = (cp << 6) | (uint32_t)(s[i] & 0x3F);
    }
    *p += extra + 1;
    return cp;
}
