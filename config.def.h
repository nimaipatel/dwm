/* See LICENSE file for copyright and license details. */ #include <X11/XF86keysym.h>

/* appearance */
static const unsigned int borderpx  = 5;        /* border pixel of windows */
static const unsigned int gappx     = 0;        /* gaps between windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 0;        /* 0 means bottom bar */
static const int user_bh            = 20;        /* 0 means that dwm will calculate bar height, >= 1 means dwm will user_bh as bar height */
static const int focusonwheel       = 0;
static const int viewontag         = 1;     /* Switch view on tag switch */
static char font[]            = "monospace:size=12";
static char dmenufont[]       = "monospace:size=8";
static const char *fonts[]          = { font, "joypixels:size=10", "Symbols Nerd Font:size=10",  };
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
const char *spcmd1[] = { "st", "-g", "200x45", "-n", "ncmpcpp", "-e", "spt", NULL };
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
	/* class        instance      title        tags mask     isfloating   monitor */
	{ "Gimp",       NULL,         NULL,        0,            1,           -1 },
	{ "fzfmenu",    NULL,         "fzfmenu",   0,            1,           -1 },
	{ NULL,         "ncmpcpp",    NULL,        SPTAG(0),     1,           -1 },
	{ NULL,         "scratch",    NULL,        SPTAG(1),     1,           -1 },
};

/* window swallowing */
static const int swaldecay = 3;
static const int swalretroactive = 1;
static const char swalsymbol[] = "👅";

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

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
	/* spawn new terminal */
	{ MODKEY,                       XK_Return,                         spawn,                 {.v = termcmd } },

	/* toggle the bar */
	{ MODKEY|ShiftMask,             XK_b,                              togglebar,             {0} },

	/* change gaps */
	{ MODKEY,                       XK_minus,                          setgaps,               {.i = -1 } },
	{ MODKEY,                       XK_equal,                          setgaps,               {.i = +1 } },
	{ MODKEY,                       XK_BackSpace,                      setgaps,               {.i = 0  } },

	/* undo window swallow */
	{ MODKEY,                       XK_x,                              swalstopsel,           {0} },

	/* TODO: handle this using signals */
	{ MODKEY,                       XK_F12,                            xrdb,                  {.v = NULL } },

	/* toggle ncmpcpp in scratchpad */
	{ MODKEY,                       XK_n,                              togglescratch,         {.ui = 0 } },

	/* toggle scratchpad */
	{ MODKEY,                       XK_slash,                          togglescratch,         {.ui = 1 } },

	/* pesudo-vim bindings for focusing next window in stack */
	{ MODKEY,                       XK_j,                              focusstack,            {.i = +1 } },
	{ MODKEY,                       XK_k,                              focusstack,            {.i = -1 } },

	/* rotating the stack */
	{ MODKEY|ShiftMask,             XK_k,                              inplacerotate,         {.i = +2} },
	{ MODKEY|ShiftMask,             XK_j,                              inplacerotate,         {.i = -2} },
	{ MODKEY|ShiftMask,             XK_l,                              inplacerotate,         {.i = +1} },
	{ MODKEY|ShiftMask,             XK_h,                              inplacerotate,         {.i = -1} },

	/* change number of windows in master frame */
	{ MODKEY,                       XK_o,                              incnmaster,            {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_o,                              incnmaster,            {.i = -1 } },

	/* modify width of master region */
	{ MODKEY,                       XK_h,                              setmfact,              {.f = -0.01} },
	{ MODKEY,                       XK_l,                              setmfact,              {.f = +0.01} },


	{ MODKEY|ShiftMask,             XK_Return,                         zoom,                  {0} },

	/* switch between previous two tags */
	{ MODKEY,                       XK_Tab,                            view,                  {0} },

	/* kill focused window */
	{ MODKEY|ShiftMask,             XK_q,                              killclient,            {0} },

	/* change layouts */
	{ MODKEY,                       XK_t,                              setlayout,             {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,                              setlayout,             {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,                              setlayout,             {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,                          setlayout,             {0} },

	/* floating windows */
	{ MODKEY|ShiftMask,             XK_space,                          togglefloating,        {0} },

	/* for multiple monitors */
	{ MODKEY,                       XK_comma,                          focusmon,              {.i = -1 } },
	{ MODKEY,                       XK_period,                         focusmon,              {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,                          tagmon,                {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period,                         tagmon,                {.i = +1 } },

	/* program launcher */
	{ MODKEY,                       XK_d,                              spawn,                 SHCMD("fzf-launcher") },

	/* password */
	{ MODKEY,                       XK_p,                              spawn,                 SHCMD("passmenu -p 'Select Password'") },
	
	/* search for unicode characters */
	{ MODKEY,                       XK_u,                              spawn,                 SHCMD("unicode-selector") },

	{ MODKEY,                       XK_e,                              spawn,                 SHCMD("$TERMINAL -e tmux new-session -As def") },

	/* search some websites for a term */
	{ MODKEY,                       XK_s,                              spawn,                 SHCMD("browser-search") },

	/* screenlock */
	{ MODKEY,                       XK_q,                              spawn,                 SHCMD("lockscreen") },

	/* get options for reboot, shutdown, et cetera*/
	{ MODKEY|ShiftMask,             XK_x,                              spawn,                 SHCMD("sys-options") },

	/* taking screenshots */
	{ 0,                            XK_Print,                          spawn,                 SHCMD("scrot --exec 'mv \"$f\" ~/Pictures/screenshots && notify-send \"Saved $f\"'") },
	{ MODKEY,                       XK_Print,                          spawn,                 SHCMD("scrot --line style=dash,width=3,color=\"red\" --select --exec 'mv \"$f\" ~/Pictures/screenshots && notify-send \"Saved $f\"'") },
	{ ShiftMask,                    XK_Print,                          spawn,                 SHCMD("scrot --exec 'xclip -selection clipboard -t image/png -i \"$f\" && mv \"$f\" ~/Pictures/screenshots && notify-send \"Saved $f and copied it to clipboard\"'") },
	{ MODKEY|ShiftMask,             XK_Print,                          spawn,                 SHCMD("scrot --line style=dash,width=3,color=\"red\" --select --exec 'xclip -selection clipboard -t image/png -i \"$f\" && mv \"$f\" ~/Pictures/screenshots && notify-send \"Saved $f and copied it to clipboard\"'") },

	/* brightness controls */
	{ 0,                            XF86XK_MonBrightnessUp,            spawn,                 SHCMD("brightnessctl set +1% ; pkill -RTMIN+19 dwmblocks") },
	{ 0,                            XF86XK_MonBrightnessDown,          spawn,                 SHCMD("brightnessctl set 1%- ; pkill -RTMIN+19 dwmblocks") },
	{ MODKEY|ShiftMask,             XK_k,                              spawn,                 SHCMD("brightnessctl set +1% ; pkill -RTMIN+19 dwmblocks") },
	{ MODKEY|ShiftMask,             XK_j,                              spawn,                 SHCMD("brightnessctl set 1%- ; pkill -RTMIN+19 dwmblocks") },

	/* audio/volume controls */
	{ 0,                            XF86XK_AudioMute,                  spawn,                 SHCMD("pamixer -t; kill -44 $(pidof dwmblocks)") },
	{ 0,                            XF86XK_AudioRaiseVolume,           spawn,                 SHCMD("pamixer --allow-boost -i 1; kill -44 $(pidof dwmblocks)") },
	{ 0,                            XF86XK_AudioLowerVolume,           spawn,                 SHCMD("pamixer --allow-boost -d 1; kill -44 $(pidof dwmblocks)") },
	{ MODKEY|ControlMask,           XK_space,                          spawn,                 SHCMD("pamixer -t; kill -44 $(pidof dwmblocks)") },
	{ MODKEY|ControlMask,           XK_k,                              spawn,                 SHCMD("pamixer --allow-boost -i 1; kill -44 $(pidof dwmblocks)") },
	{ MODKEY|ControlMask,           XK_j,                              spawn,                 SHCMD("pamixer --allow-boost -d 1; kill -44 $(pidof dwmblocks)") },

	/* music controls */
	{ 0,                            XF86XK_AudioPrev,                  spawn,                 SHCMD("mpc prev") },
	{ 0,                            XF86XK_AudioNext,                  spawn,                 SHCMD("mpc next") },
	{ 0,                            XF86XK_AudioPlay,                  spawn,                 SHCMD("mpc toggle") },

	/* switch between and activate tags (0 is for all) */
	{ MODKEY,                       XK_0,                              view,                  {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,                              tag,                   {.ui = ~0 } },
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

