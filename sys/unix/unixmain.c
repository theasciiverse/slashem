/*	SCCS Id: @(#)unixmain.c	3.4	1997/01/22	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* main.c - Unix NetHack */

/* Changed for the Asciiverse, 2026-11-16: with ASCIIVERSE defined, the
 * playground is the player's own folder, slashem under $XDG_STATE_HOME
 * (else ~/.local/state), made on the first run, and the read-only data
 * (nhshare, nhushare, license) is read from the folder the binary is in;
 * see av_playground() and av_dataprefix() below. */

#include "hack.h"
#include "dlb.h"

#include <sys/stat.h>
#include <signal.h>
#include <pwd.h>
#ifndef O_RDONLY
#include <fcntl.h>
#endif
#ifdef ASCIIVERSE
#include <unistd.h>
#include <fcntl.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#endif

#if !defined(_BULL_SOURCE) && !defined(__sgi) && !defined(_M_UNIX)
# if !defined(SUNOS4) && !(defined(ULTRIX) && defined(__GNUC__))
#  if defined(POSIX_TYPES) || defined(SVR4) || defined(HPUX)
extern struct passwd *FDECL(getpwuid,(uid_t));
#  else
extern struct passwd *FDECL(getpwuid,(int));
#  endif
# endif
#endif
extern struct passwd *FDECL(getpwnam,(const char *));
#ifdef CHDIR
static void FDECL(chdirx, (const char *,BOOLEAN_P));
#endif /* CHDIR */
static boolean NDECL(whoami);
static void FDECL(process_options, (int, char **));
#ifdef ASCIIVERSE
static char *NDECL(av_playground);
static void FDECL(av_dataprefix, (const char *, const char *));
#endif

#ifdef _M_UNIX
extern void NDECL(check_sco_console);
extern void NDECL(init_sco_cons);
#endif
#ifdef __linux__
extern void NDECL(check_linux_console);
extern void NDECL(init_linux_cons);
#endif

#ifdef LINUX
extern void NDECL(check_linux_console);
extern void NDECL(init_linux_cons);
#endif
 
static void NDECL(wd_message);
#ifdef WIZARD
static boolean wiz_error_flag = FALSE;
#endif

int
main(argc,argv)
int argc;
char *argv[];
{
	register int fd;
#ifdef CHDIR
	register char *dir;
#endif
	boolean exact_username;

	hname = argv[0];
	hackpid = getpid();
	(void) umask(0777 & ~FCMASK);

	choose_windows(DEFAULT_WINDOW_SYS);

#ifdef CHDIR			/* otherwise no chdir() */
	/*
	 * See if we must change directory to the playground.
	 * (Perhaps hack runs suid and playground is inaccessible
	 *  for the player.)
	 * The environment variable HACKDIR is overridden by a
	 *  -d command line option (must be the first option given)
	 */
	dir = nh_getenv("NETHACKDIR");
	if (!dir) dir = nh_getenv("HACKDIR");
#endif
#ifdef ASCIIVERSE
	{
	    char startdir[BUFSZ];

	    if (!getcwd(startdir, sizeof startdir))
		startdir[0] = '\0';
	    av_dataprefix(argv[0], startdir);
	}
	if (!dir &&
		!(argc > 1 && !strncmp(argv[1], "-d", 2) && argv[1][2] != 'e')) {
	    /* folders get the usual 0755, not the game's file mask */
	    int mask = umask(022);

	    dir = av_playground();
	    (void) umask(mask);
	}
#endif
	if(argc > 1) {
#ifdef CHDIR
	    if (!strncmp(argv[1], "-d", 2) && argv[1][2] != 'e') {
		/* avoid matching "-dec" for DECgraphics; since the man page
		 * says -d directory, hope nobody's using -desomething_else
		 */
		argc--;
		argv++;
		dir = argv[0]+2;
		if(*dir == '=' || *dir == ':') dir++;
		if(!*dir && argc > 1) {
			argc--;
			argv++;
			dir = argv[0];
		}
		if(!*dir)
		    error("Flag -d must be followed by a directory name.");
	    }
	    if (argc > 1)
#endif /* CHDIR */

	    /*
	     * Now we know the directory containing 'record' and
	     * may do a prscore().  Exclude `-style' - it's a Qt option.
	     */
	    if (!strncmp(argv[1], "-s", 2) && strncmp(argv[1], "-style", 6)) {
#ifdef CHDIR
		chdirx(dir,0);
#endif
		prscore(argc, argv);
		exit(EXIT_SUCCESS);
	    }
	}

	/*
	 * Change directories before we initialize the window system so
	 * we can find the tile file.
	 */
#ifdef CHDIR
	chdirx(dir,1);
#endif

#ifdef _M_UNIX
	check_sco_console();
#endif
#ifdef LINUX
	check_linux_console();
#endif
#ifdef PROXY_GRAPHICS
	/* Handle --proxy before options, if supported */
	if (argc > 1 && !strcmp(argv[1], "--proxy")) {
	    argv[1] = argv[0];
	    argc--;
	    argv++;
	    choose_windows("proxy");
	    lock_windows(TRUE);         /* Can't be overridden from options */
	}
#endif
	initoptions();
	init_nhwindows(&argc,argv);
	exact_username = whoami();
#ifdef _M_UNIX
	init_sco_cons();
#endif
#ifdef LINUX
	init_linux_cons();
#endif
	/*
	 * It seems you really want to play.
	 */
	u.uhp = 1;	/* prevent RIP on early quits */
	(void) signal(SIGHUP, (SIG_RET_TYPE) hangup);
#ifdef SIGXCPU
	(void) signal(SIGXCPU, (SIG_RET_TYPE) hangup);
#endif
#ifdef SIGPIPE		/* eg., a lost proxy connection */
	(void) signal(SIGPIPE, (SIG_RET_TYPE) hangup);
#endif

	process_options(argc, argv);	/* command line options */

#ifdef DEF_PAGER
	if(!(catmore = nh_getenv("HACKPAGER")) && !(catmore = nh_getenv("PAGER")))
		catmore = DEF_PAGER;
#endif
#ifdef MAIL
	getmailstatus();
#endif
#ifdef WIZARD
	if (wizard)
		Strcpy(plname, "wizard");
	else
#endif
	if(!*plname || !strncmp(plname, "player", 4)
		    || !strncmp(plname, "games", 4)) {
		askname();
	} else if (exact_username) {
		/* guard against user names with hyphens in them */
		int len = strlen(plname);
		/* append the current role, if any, so that last dash is ours */
		if (++len < sizeof plname)
			(void)strncat(strcat(plname, "-"),
				      pl_character, sizeof plname - len - 1);
	}
	plnamesuffix();		/* strip suffix from name; calls askname() */
				/* again if suffix was whole name */
				/* accepts any suffix */
#ifdef WIZARD
	if(!wizard) {
#endif
		/*
		 * check for multiple games under the same name
		 * (if !locknum) or check max nr of players (otherwise)
		 */
		(void) signal(SIGQUIT,SIG_IGN);
		(void) signal(SIGINT,SIG_IGN);
		if(!locknum)
			Sprintf(lock, "%d%s", (int)getuid(), plname);
		getlock();
#ifdef WIZARD
	} else {
		Sprintf(lock, "%d%s", (int)getuid(), plname);
		getlock();
	}
#endif /* WIZARD */

	dlb_init();	/* must be before newgame() */

	/*
	 * Initialization of the boundaries of the mazes
	 * Both boundaries have to be even.
	 */
	x_maze_max = COLNO-1;
	if (x_maze_max % 2)
		x_maze_max--;
	y_maze_max = ROWNO-1;
	if (y_maze_max % 2)
		y_maze_max--;

	/*
	 *  Initialize the vision system.  This must be before mklev() on a
	 *  new game or before a level restore on a saved game.
	 */
	vision_init();

	display_gamewindows();

	if ((fd = restore_saved_game()) >= 0) {
#ifdef WIZARD
		/* Since wizard is actually flags.debug, restoring might
		 * overwrite it.
		 */
		boolean remember_wiz_mode = wizard;
#endif
#ifndef FILE_AREAS
		const char *fq_save = fqname(SAVEF, SAVEPREFIX, 1);

		(void) chmod(fq_save,0);	/* disallow parallel restores */
#else
		(void) chmod_area(FILE_AREA_SAVE, SAVEF, 0);
#endif
		(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#ifdef NEWS
		if(iflags.news) {
		    display_file_area(NEWS_AREA, NEWS, FALSE);
		    iflags.news = FALSE; /* in case dorecover() fails */
		}
#endif
		pline("Restoring save file...");
		mark_synch();	/* flush output */
		if(!dorecover(fd))
			goto not_recovered;
#ifdef WIZARD
		if(!wizard && remember_wiz_mode) wizard = TRUE;
#endif
		check_special_room(FALSE);
		wd_message();

		if (discover || wizard) {
			if(yn("Do you want to keep the save file?") == 'n')
			    (void) delete_savefile();
			else {
#ifndef FILE_AREAS
			    (void) chmod(fq_save,FCMASK); /* back to readable */
			    compress_area(NULL, fq_save);
#else
			    (void) chmod_area(FILE_AREA_SAVE, SAVEF, FCMASK);
			    compress_area(FILE_AREA_SAVE, SAVEF);
#endif
			}
		}
		flags.move = 0;
	} else {
not_recovered:
		player_selection();
		newgame();
		wd_message();

		flags.move = 0;
		set_wear();
		(void) pickup(1);
	}

	moveloop();
	exit(EXIT_SUCCESS);
	/*NOTREACHED*/
	return(0);
}

static void
process_options(argc, argv)
int argc;
char *argv[];
{
	int i;


	/*
	 * Process options.
	 */
	while(argc > 1 && argv[1][0] == '-'){
		argv++;
		argc--;
		switch(argv[0][1]){
		case 'D':
		case 'Z':
#ifdef WIZARD
			{
			  char *user;
			  int uid;
			  struct passwd *pw = (struct passwd *)0;

			  uid = getuid();
			  user = getlogin();
			  if (user) {
			      pw = getpwnam(user);
			      if (pw && (pw->pw_uid != uid)) pw = 0;
			  }
			  if (pw == 0) {
			      user = nh_getenv("USER");
			      if (user) {
				  pw = getpwnam(user);
				  if (pw && (pw->pw_uid != uid)) pw = 0;
			      }
			      if (pw == 0) {
				  pw = getpwuid(uid);
			      }
			  }
			  if (pw && !strcmp(pw->pw_name,WIZARD)) {
			      wizard = TRUE;
			      break;
			  }
			}
			/* otherwise fall thru to discover */
			wiz_error_flag = TRUE;
#endif
		case 'X':
			discover = TRUE;
			break;
#ifdef NEWS
		case 'n':
			iflags.news = FALSE;
			break;
#endif
		case 'u':
			if(argv[0][2])
			  (void) strncpy(plname, argv[0]+2, sizeof(plname)-1);
			else if(argc > 1) {
			  argc--;
			  argv++;
			  (void) strncpy(plname, argv[0], sizeof(plname)-1);
			} else
				raw_print("Player name expected after -u");
			break;
		case 'i':
			if (!strncmpi(argv[0]+1, "IBM", 3))
				switch_graphics(IBM_GRAPHICS);
			break;
		case 'd':
			if (!strncmpi(argv[0]+1, "DEC", 3))
				switch_graphics(DEC_GRAPHICS);
			break;
		case 'p': /* profession (role) */
			if (argv[0][2]) {
			    if ((i = str2role(&argv[0][2])) >= 0)
			    	flags.initrole = i;
			} else if (argc > 1) {
				argc--;
				argv++;
			    if ((i = str2role(argv[0])) >= 0)
			    	flags.initrole = i;
			}
			break;
		case 'r': /* race */
			if (argv[0][2]) {
			    if ((i = str2race(&argv[0][2])) >= 0)
			    	flags.initrace = i;
			} else if (argc > 1) {
				argc--;
				argv++;
			    if ((i = str2race(argv[0])) >= 0)
			    	flags.initrace = i;
			}
			break;
		case 'g': /* gender */
			if (argv[0][2]) {
			    if ((i = str2gend(&argv[0][2])) >= 0)
			    	flags.initgend = i;
			} else if (argc > 1) {
				argc--;
				argv++;
			    if ((i = str2gend(argv[0])) >= 0)
			    	flags.initgend = i;
			}
			break;
		case 'a': /* align */
			if (argv[0][2]) {
			    if ((i = str2align(&argv[0][2])) >= 0)
			    	flags.initalign = i;
			} else if (argc > 1) {
				argc--;
				argv++;
			    if ((i = str2align(argv[0])) >= 0)
			    	flags.initalign = i;
			}
			break;
		case '@':
			flags.randomall = 1;
			break;
		default:
			if ((i = str2role(&argv[0][1])) >= 0) {
			    flags.initrole = i;
				break;
			}
			/* else raw_printf("Unknown option: %s", *argv); */
		}
	}

	if(argc > 1)
		locknum = atoi(argv[1]);
#ifdef MAX_NR_OF_PLAYERS
	if(!locknum || locknum > MAX_NR_OF_PLAYERS)
		locknum = MAX_NR_OF_PLAYERS;
#endif
}

#ifdef CHDIR
static void
chdirx(dir, wr)
const char *dir;
boolean wr;
{
	if (dir					/* User specified directory? */
# ifdef HACKDIR
	       && strcmp(dir, HACKDIR)		/* and not the default? */
# endif
		) {
# ifdef SECURE
	    (void) setgid(getgid());
	    (void) setuid(getuid());		/* Ron Wessels */
# endif
	} else {
	    /* non-default data files is a sign that scores may not be
	     * compatible, or perhaps that a binary not fitting this
	     * system's layout is being used.
	     */
# ifdef VAR_PLAYGROUND
	    int len = strlen(VAR_PLAYGROUND);

	    fqn_prefix[SCOREPREFIX] = (char *)alloc(len+2);
	    Strcpy(fqn_prefix[SCOREPREFIX], VAR_PLAYGROUND);
	    if (fqn_prefix[SCOREPREFIX][len-1] != '/') {
		fqn_prefix[SCOREPREFIX][len] = '/';
		fqn_prefix[SCOREPREFIX][len+1] = '\0';
	    }
# endif
	}

# ifdef HACKDIR
	if (dir == (const char *)0)
	    dir = HACKDIR;
# endif

	if (dir && chdir(dir) < 0) {
	    perror(dir);
	    error("Cannot chdir to %s.", dir);
	}

	/* warn the player if we can't write the record file */
	/* perhaps we should also test whether . is writable */
	/* unfortunately the access system-call is worthless */
	if (wr) {
# ifdef VAR_PLAYGROUND
	    fqn_prefix[LEVELPREFIX] = fqn_prefix[SCOREPREFIX];
	    fqn_prefix[SAVEPREFIX] = fqn_prefix[SCOREPREFIX];
	    fqn_prefix[BONESPREFIX] = fqn_prefix[SCOREPREFIX];
	    fqn_prefix[LOCKPREFIX] = fqn_prefix[SCOREPREFIX];
	    fqn_prefix[TROUBLEPREFIX] = fqn_prefix[SCOREPREFIX];
# endif
	    check_recordfile(dir);
	}
}
#endif /* CHDIR */

#ifdef ASCIIVERSE
/* Makes PATH (a folder) if it is missing; FALSE if it cannot. */
static boolean
av_mkdir(path)
const char *path;
{
	struct stat st;

	if (stat(path, &st) == 0)
	    return S_ISDIR(st.st_mode);
	return mkdir(path, 0755) == 0;
}

/* Makes an empty file NAME in the current folder if it is missing. */
static void
av_touch(name)
const char *name;
{
	int fd = open(name, O_WRONLY | O_CREAT, FCMASK);

	if (fd >= 0)
	    (void) close(fd);
}

/* Writes a first options file, slashemrc, if there is none: colour on,
 * which SLASH'EM leaves off on Unix, so the dungeon shows its colours
 * from the first game. ~/.slashemrc, when there is one, is read instead
 * (src/files.c). */
static void
av_options()
{
	FILE *fp;

	if (access("slashemrc", F_OK) == 0)
	    return;
	if ((fp = fopen("slashemrc", "w")) == (FILE *) 0)
	    return;
	fputs("# SLASH'EM options, read when there is no ~/.slashemrc.\n", fp);
	fputs("# One OPTIONS= line per option; see the game's help (?) for\n", fp);
	fputs("# the full list, e.g. OPTIONS=name:Freya,role:Valkyrie\n", fp);
	fputs("OPTIONS=color\n", fp);
	(void) fclose(fp);
}

/* The player's playground: $XDG_STATE_HOME/slashem, else
 * ~/.local/state/slashem, made with its save folder on the first run.
 * The record, logfile and perm files must exist before the game locks
 * them by linking to them, so they are made here too. */
static char *
av_playground()
{
	static char path[BUFSZ];
	char here[BUFSZ];
	const char *xdg = nh_getenv("XDG_STATE_HOME");
	const char *home = nh_getenv("HOME");
	char *p;

	if (xdg && *xdg == '/' && strlen(xdg) < BUFSZ - 20)
	    Sprintf(path, "%s/slashem", xdg);
	else if (home && *home && strlen(home) < BUFSZ - 32)
	    Sprintf(path, "%s/.local/state/slashem", home);
	else
	    return (char *) 0;	/* stay where we are */
	/* make each missing folder on the way */
	for (p = path + 1; *p; p++)
	    if (*p == '/') {
		*p = '\0';
		(void) av_mkdir(path);
		*p = '/';
	    }
	if (!av_mkdir(path))
	    return (char *) 0;
	if (getcwd(here, sizeof here) && chdir(path) == 0) {
	    (void) av_mkdir("save");
	    av_touch(NH_RECORD);
	    av_touch(LOGFILE);
	    av_touch(HLOCK);
	    av_options();
	    (void) chdir(here);
	}
	return path;
}

/* TRUE if FOLDER holds the data library. */
static boolean
av_has_data(folder)
const char *folder;
{
	char name[BUFSZ];

	if (!*folder || strlen(folder) + strlen(DLBFILE) + 2 > sizeof name)
	    return FALSE;
	Sprintf(name, "%s/%s", folder, DLBFILE);
	return access(name, R_OK) == 0;
}

/* Reads the shipped data (nhshare, nhushare, license) from the folder
 * the binary is in, else from the folder the game was started in (av
 * starts a title in its install folder), else from the playground as
 * before. */
static void
av_dataprefix(argv0, startdir)
const char *argv0, *startdir;
{
	char exe[BUFSZ], *slash;
	const char *found = (const char *) 0;
	boolean have_exe = FALSE;
#if defined(__APPLE__)
	char raw[BUFSZ];
	uint32_t size = sizeof raw;

	if (_NSGetExecutablePath(raw, &size) == 0 && realpath(raw, exe))
	    have_exe = TRUE;
#elif defined(__linux__)
	ssize_t n = readlink("/proc/self/exe", exe, sizeof exe - 1);

	if (n > 0) {
	    exe[n] = '\0';
	    have_exe = TRUE;
	}
#endif
	if (!have_exe && argv0 && index(argv0, '/')
		&& strlen(startdir) + strlen(argv0) + 2 < sizeof exe) {
	    if (*argv0 == '/')
		Strcpy(exe, argv0);
	    else
		Sprintf(exe, "%s/%s", startdir, argv0);
	    have_exe = TRUE;
	}
	if (have_exe && (slash = rindex(exe, '/')) != 0) {
	    *slash = '\0';
	    if (av_has_data(exe))
		found = exe;
	}
	if (!found && av_has_data(startdir))
	    found = startdir;
	if (found) {
	    fqn_prefix[DATAPREFIX] = (char *) alloc(strlen(found) + 2);
	    Sprintf(fqn_prefix[DATAPREFIX], "%s/", found);
	}
}
#endif /* ASCIIVERSE */

static boolean
whoami() {
	/*
	 * Who am i? Algorithm: 1. Use name as specified in NETHACKOPTIONS
	 *			2. Use $USER or $LOGNAME	(if 1. fails)
	 *			3. Use getlogin()		(if 2. fails)
	 * The resulting name is overridden by command line options.
	 * If everything fails, or if the resulting name is some generic
	 * account like "games", "play", "player", "hack" then eventually
	 * we'll ask him.
	 * Note that we trust the user here; it is possible to play under
	 * somebody else's name.
	 */
	register char *s;

	if (*plname) return FALSE;
	if(/* !*plname && */ (s = nh_getenv("USER")))
		(void) strncpy(plname, s, sizeof(plname)-1);
	if(!*plname && (s = nh_getenv("LOGNAME")))
		(void) strncpy(plname, s, sizeof(plname)-1);
	if(!*plname && (s = getlogin()))
		(void) strncpy(plname, s, sizeof(plname)-1);
	return TRUE;
}

#ifdef PORT_HELP
void
port_help()
{
	/*
	 * Display unix-specific help.   Just show contents of the helpfile
	 * named by PORT_HELP.
	 */
	display_file_area(FILE_AREA_SHARE, PORT_HELP, TRUE);
}
#endif

static void
wd_message()
{
#ifdef WIZARD
	if (wiz_error_flag) {
		pline("Only user \"%s\" may access debug (wizard) mode.",
# ifndef KR1ED
			WIZARD);
# else
			WIZARD_NAME);
# endif
		pline("Entering discovery mode instead.");
	} else
#endif
	if (discover)
		You("are in non-scoring discovery mode.");
}

/*
 * Add a slash to any name not ending in /. There must
 * be room for the /
 */
void
append_slash(name)
char *name;
{
	char *ptr;

	if (!*name)
		return;
	ptr = name + (strlen(name) - 1);
	if (*ptr != '/') {
		*++ptr = '/';
		*++ptr = '\0';
	}
	return;
}

/*unixmain.c*/
