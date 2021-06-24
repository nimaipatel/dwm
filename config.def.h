/* See LICENSE file for copyright and license details. */ #include <X11/XF86keysym.h>

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int gappx     = 0;        /* gaps between windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 0;        /* 0 means bottom bar */
static const int user_bh            = 20;        /* 0 means that dwm will calculate bar height, >= 1 means dwm will user_bh as bar height */
static const int focusonwheel       = 0;
static const int viewontag         = 1;     /* Switch view on tag switch */
static char font[]            = "monospace:size=8";
static char dmenufont[]       = "monospace:size=8";
static const char *fonts[]          = { font, "joypixels:size=8", "Symbols Nerd Font:size=8",  };
static char normbgcolor[]           = "#222222";
static char normbordercolor[]       = "#444444";
static char normfgcolor[]           = "#bbbbbb";
static char selfgcolor[]            = "#eeeeee";
static char selbordercolor[]        = "#005577";
static char selbgcolor[]            = "#005577";
static char *colors[][3] = {
       /*               fg           bg           border   */
       [SchemeNorm] = { normfgcolor, normbgcolor, normbordercolor },
       [SchemeSel]  = { selfgcolor,  selbgcolor,  selbordercolor  },
};

typedef struct {
	const char *name;
	const void *cmd;
} Sp;
const char *spcmd1[] = { "st", "-g", "120x40", "-n", "ncmpcpp", "-e", "ncmpcpp", NULL };
const char *spcmd2[] = { "st", "-g", "120x40", "-n", "scratch", NULL };
static Sp scratchpads[] = {
	/* name          cmd  */
	{"ncmpcpp",      spcmd1},
	{"scratch",      spcmd2},
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance      title    tags mask     isfloating   monitor */
	{ "Gimp",       NULL,         NULL,      0,            1,           -1 },
	{ NULL,         "ncmpcpp",    NULL,      SPTAG(0),     1,           -1 },
	{ NULL,         "scratch",    NULL,      SPTAG(1),     1,           -1 },
};

/* window swallowing */
static const int swaldecay = 3;
static const int swalretroactive = 1;
static const char swalsymbol[] = "👅";

/* layout(s) */
static float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static int nmaster     = 1;    /* number of clients in master area */
static int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbordercolor, "-sf", selfgcolor, NULL };
static const char *termcmd[]  = { "st", NULL };

/*
 * Xresources preferences to load at startup
 */
ResourcePref resources[] = {
		{ "font",               STRING,  &font },
		{ "dmenufont",          STRING,  &dmenufont },
		{ "normbgcolor",        STRING,  &normbgcolor },
		{ "normbordercolor",    STRING,  &normbordercolor },
		{ "normfgcolor",        STRING,  &normfgcolor },
		{ "selbgcolor",         STRING,  &selbgcolor },
		{ "selbordercolor",     STRING,  &selbordercolor },
		{ "selfgcolor",         STRING,  &selfgcolor },
		{ "borderpx",          	INTEGER, &borderpx },
		{ "snap",          		INTEGER, &snap },
		{ "showbar",          	INTEGER, &showbar },
		{ "topbar",          	INTEGER, &topbar },
		{ "nmaster",          	INTEGER, &nmaster },
		{ "resizehints",       	INTEGER, &resizehints },
		{ "mfact",      	 	FLOAT,   &mfact },
};

static Key keys[] = {
	/* modifier                     key                                function               argument */
	{ MODKEY,                       XK_Return,                         spawn,                 {.v = termcmd } },
	{ MODKEY|ShiftMask,             XK_b,                              togglebar,             {0} },
	{ MODKEY,                       XK_minus,                          setgaps,               {.i = -1 } },
	{ MODKEY,                       XK_equal,                          setgaps,               {.i = +1 } },
	{ MODKEY,                       XK_BackSpace,                      setgaps,               {.i = 0  } },
	{ MODKEY,                       XK_x,                              swalstopsel,           {0} },
	{ MODKEY,                       XK_F12,                            xrdb,                  {.v = NULL } },
	{ MODKEY,                       XK_n,                              togglescratch,         {.ui = 0 } },
	{ MODKEY,                       XK_slash,                          togglescratch,         {.ui = 1 } },
	{ MODKEY,                       XK_j,                              focusstack,            {.i = +1 } },
	{ MODKEY,                       XK_k,                              focusstack,            {.i = -1 } },
	{ MODKEY,                       XK_o,                              incnmaster,            {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_o,                              incnmaster,            {.i = -1 } },
	{ MODKEY,                       XK_h,                              setmfact,              {.f = -0.01} },
	{ MODKEY,                       XK_l,                              setmfact,              {.f = +0.01} },
	{ MODKEY|ShiftMask,             XK_Return,                         zoom,                  {0} },
	{ MODKEY,                       XK_Tab,                            view,                  {0} },
	{ MODKEY|ShiftMask,             XK_q,                              killclient,            {0} },
	{ MODKEY,                       XK_t,                              setlayout,             {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,                              setlayout,             {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,                              setlayout,             {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,                          setlayout,             {0} },
	{ MODKEY|ShiftMask,             XK_space,                          togglefloating,        {0} },
	{ MODKEY,                       XK_0,                              view,                  {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,                              tag,                   {.ui = ~0 } },
	{ MODKEY,                       XK_comma,                          focusmon,              {.i = -1 } },
	{ MODKEY,                       XK_period,                         focusmon,              {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,                          tagmon,                {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period,                         tagmon,                {.i = +1 } },
	{ MODKEY,                       XK_d,                              spawn,                 SHCMD("dmenu_run -l 30") },
	{ MODKEY,                       XK_p,                              spawn,                 SHCMD("passmenu -i -l 30") },
	{ MODKEY,                       XK_u,                              spawn,                 SHCMD("dmenu-emojis") },
	{ MODKEY|ShiftMask,             XK_m,                              spawn,                 SHCMD("dmenu-mount") },
	{ MODKEY|ShiftMask,             XK_u,                              spawn,                 SHCMD("dmenu-unmount") },
	{ MODKEY|ShiftMask,             XK_w,                              spawn,                 SHCMD("dmenu-wallpaper") },
	{ MODKEY,                       XK_y,                              spawn,                 SHCMD("launch-tmux-dmenu") },
	{ MODKEY,                       XK_e,                              spawn,                 SHCMD("dmenu-change-mode") },
	{ MODKEY|ShiftMask,             XK_e,                              spawn,                 SHCMD("dmenu-change-font") },
	{ MODKEY,                       XK_b,                              spawn,                 SHCMD("$BROWSER") },
	{ MODKEY,                       XK_r,                              spawn,                 SHCMD("$TERMINAL -e $TERMINAL_FILE_BROWSER") },
	{ MODKEY|ShiftMask,             XK_r,                              spawn,                 SHCMD("$GUI_FILE_BROWSER") },
	{ MODKEY,                       XK_w,                              spawn,                 SHCMD("dmenu-wifi") },
	{ MODKEY,                       XK_q,                              spawn,                 SHCMD("lockscreen") },
	{ MODKEY|ShiftMask,             XK_p,                              spawn,                 SHCMD("killall picom || picom") },
	{ MODKEY|ShiftMask,             XK_x,                              spawn,                 SHCMD("dmenu-sys-options") },
	{ 0,                            XK_Print,                          spawn,                 SHCMD("scrot --exec 'mv \"$f\" ~/pictures/screenshots && notify-send \"Saved $f\"'") },
	{ MODKEY,                       XK_Print,                          spawn,                 SHCMD("scrot --line style=dash,width=3,color=\"red\" --select --exec 'mv \"$f\" ~/pictures/screenshots && notify-send \"Saved $f\"'") },
	{ ShiftMask,                    XK_Print,                          spawn,                 SHCMD("scrot --exec 'xclip -selection clipboard -t image/png -i \"$f\" && mv \"$f\" ~/pictures/screenshots && notify-send \"Saved $f and copied it to clipboard\"'") },
	{ MODKEY|ShiftMask,             XK_Print,                          spawn,                 SHCMD("scrot --line style=dash,width=3,color=\"red\" --select --exec 'xclip -selection clipboard -t image/png -i \"$f\" && mv \"$f\" ~/pictures/screenshots && notify-send \"Saved $f and copied it to clipboard\"'") },
	{ 0,                            XF86XK_MonBrightnessUp,            spawn,                 SHCMD("brightnessctl set +2%") },
	{ 0,                            XF86XK_MonBrightnessDown,          spawn,                 SHCMD("brightnessctl set 2%-") },
	{ MODKEY|ShiftMask,             XK_k,                              spawn,                 SHCMD("brightnessctl set +2%") },
	{ MODKEY|ShiftMask,             XK_j,                              spawn,                 SHCMD("brightnessctl set 2%-") },
	{ 0,                            XF86XK_AudioMute,                  spawn,                 SHCMD("pamixer -t; kill -44 $(pidof dwmblocks)") },
	{ 0,                            XF86XK_AudioRaiseVolume,           spawn,                 SHCMD("pamixer --allow-boost -i 1; kill -44 $(pidof dwmblocks)") },
	{ 0,                            XF86XK_AudioLowerVolume,           spawn,                 SHCMD("pamixer --allow-boost -d 1; kill -44 $(pidof dwmblocks)") },
	{ 0,                            XF86XK_AudioPrev,                  spawn,                 SHCMD("mpc prev") },
	{ 0,                            XF86XK_AudioNext,                  spawn,                 SHCMD("mpc next") },
	{ 0,                            XF86XK_AudioPlay,                  spawn,                 SHCMD("mpc toggle") },
	{ MODKEY|ControlMask,           XK_space,                          spawn,                 SHCMD("pamixer -t; kill -44 $(pidof dwmblocks)") },
	{ MODKEY|ControlMask,           XK_k,                              spawn,                 SHCMD("pamixer --allow-boost -i 1; kill -44 $(pidof dwmblocks)") },
	{ MODKEY|ControlMask,           XK_j,                              spawn,                 SHCMD("pamixer --allow-boost -d 1; kill -44 $(pidof dwmblocks)") },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkClientWin,         MODKEY|ShiftMask, Button1,      swalmouse,      {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

