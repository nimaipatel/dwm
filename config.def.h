/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>

/* appearance */
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const unsigned int gappx     = 0;        /* gaps between windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 0;        /* 0 means bottom bar */
static const int user_bh            = 30;        /* 0 means that dwm will calculate bar height, >= 1 means dwm will user_bh as bar height */
static const int focusonwheel       = 0;
static const int viewontag         = 1;     /* Switch view on tag switch */
static char font[]            = "sans-serif:size=14";
static char dmenufont[]       = "sans-serif:size=14";
static const char *fonts[]          = { font, "joypixels:size=10", "Symbols Nerd Font:size=14",  };
static char normbgcolor[]           = "#222222";
static char normbordercolor[]       = "#444444";
static char normfgcolor[]           = "#bbbbbb";
static char selfgcolor[]            = "#eeeeee";
static char selbordercolor[]        = "#005577";
static char selbgcolor[]            = "#005577";
static char termcol0[] = "#000000"; /* black   */
static char termcol1[] = "#ff0000"; /* red     */
static char termcol2[] = "#33ff00"; /* green   */
static char termcol3[] = "#ff0099"; /* yellow  */
static char termcol4[] = "#0066ff"; /* blue    */
static char termcol5[] = "#cc00ff"; /* magenta */
static char termcol6[] = "#00ffff"; /* cyan    */
static char termcol7[] = "#d0d0d0"; /* white   */
static char termcol8[]  = "#808080"; /* black   */
static char termcol9[]  = "#ff0000"; /* red     */
static char termcol10[] = "#33ff00"; /* green   */
static char termcol11[] = "#ff0099"; /* yellow  */
static char termcol12[] = "#0066ff"; /* blue    */
static char termcol13[] = "#cc00ff"; /* magenta */
static char termcol14[] = "#00ffff"; /* cyan    */
static char termcol15[] = "#ffffff"; /* white   */
static char *termcolor[] = {
	termcol0,
	termcol1,
	termcol2,
	termcol3,
	termcol4,
	termcol5,
	termcol6,
	termcol7,
	termcol8,
	termcol9,
	termcol10,
	termcol11,
	termcol12,
	termcol13,
	termcol14,
	termcol15,
};
static char *colors[][3] = {
       /*               fg           bg           border   */
       [SchemeNorm] = { normfgcolor, normbgcolor, normbordercolor },
       [SchemeSel]  = { selfgcolor,  selbgcolor,  selbordercolor  },
};

typedef struct {
	const char *name;
	const void *cmd;
} Sp;
const char *spcmd1[] = { "st", "-g", "120x30", "-n", "mixer", "-e", "pulsemixer", NULL };
const char *spcmd2[] = { "st", "-g", "120x40", "-n", "scratch", NULL };
static Sp scratchpads[] = {
	/* name          cmd  */
	{"mixer",      spcmd1},
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
	{ "scrcpy",     NULL,         NULL,        0,            1,           -1 },
	{ NULL,         "mixer",      NULL,        SPTAG(0),     1,           -1 },
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

#include "grid.c"
static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
	{ "HHH",      grid },
	{ "TTT",      bstack },
	{ "===",      bstackhoriz },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG)												\
	&((Keychord){1, {{MODKEY, KEY}},								view,           {.ui = 1 << TAG} }), \
		&((Keychord){1, {{MODKEY|ControlMask, KEY}},					toggleview,     {.ui = 1 << TAG} }), \
		&((Keychord){1, {{MODKEY|ShiftMask, KEY}},						tag,            {.ui = 1 << TAG} }), \
		&((Keychord){1, {{MODKEY|ControlMask|ShiftMask, KEY}},			toggletag,      {.ui = 1 << TAG} }),

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbordercolor, "-sf", selfgcolor, NULL };
static const char *termcmd[]  = { "st", NULL };

static Keychord *keychords[] = {
	/* spawn new terminal */
	&((Keychord){1, {{ MODKEY, XK_Return }},                                               spawn,                 SHCMD("$TERMINAL") }),

	/* spawn new browser session */
	&((Keychord){2, {{ MODKEY, XK_b }, { MODKEY, XK_b }},                                  spawn,                 SHCMD("$BROWSER --profile-directory=clean") }),
	&((Keychord){2, {{ MODKEY, XK_b }, { MODKEY, XK_m }},                                  spawn,                 SHCMD("$BROWSER --profile-directory=google-main") }),
	&((Keychord){2, {{ MODKEY, XK_b }, { MODKEY, XK_a }},                                  spawn,                 SHCMD("$BROWSER --profile-directory=google-alt") }),
	&((Keychord){2, {{ MODKEY, XK_b }, { MODKEY, XK_n }},                                  spawn,                 SHCMD("$BROWSER --incognito") }),

	&((Keychord){2, {{ MODKEY, XK_w }, { MODKEY, XK_w }},                                  spawn,                 SHCMD("lowriter --nologo") }),
	&((Keychord){2, {{ MODKEY, XK_w }, { MODKEY, XK_e }},                                  spawn,                 SHCMD("localc --nologo") }),
	&((Keychord){2, {{ MODKEY, XK_w }, { MODKEY, XK_p }},                                  spawn,                 SHCMD("loimpress --nologo") }),

	/* toggle the bar */
	&((Keychord){1, {{ MODKEY|ShiftMask, XK_b }},                                          togglebar,             {0} }),

	/* change gaps */
	&((Keychord){1, {{ MODKEY, XK_minus }},                                                setgaps,               {.i = -1 } }),
	&((Keychord){1, {{ MODKEY, XK_equal }},                                                setgaps,               {.i = +1 } }),
	&((Keychord){1, {{ MODKEY, XK_BackSpace }},                                            setgaps,               {.i = 0  } }),

	/* undo window swallow */
	&((Keychord){1, {{ MODKEY, XK_x }},                                                    swalstopsel,           {0} }),

	/* TODO: handle this using signals */
	&((Keychord){1, {{ MODKEY, XK_F12 }},                                                  xrdb,                  {.v = NULL } }),

	/* toggle pulsemixer in scratchpad */
	&((Keychord){1, {{ MODKEY, XK_m }},                                                    togglescratch,         {.ui = 0 } }),

	/* toggle scratchpad */
	&((Keychord){1, {{ MODKEY, XK_slash }},                                                togglescratch,         {.ui = 1 } }),

	/* pesudo-vim bindings for focusing next window in stack */
	&((Keychord){1, {{ MODKEY, XK_j }},                                                    focusstack,            {.i = +1 } }),
	&((Keychord){1, {{ MODKEY, XK_k }},                                                    focusstack,            {.i = -1 } }),

	/* rotating the stack */
	&((Keychord){1, {{ MODKEY|ShiftMask, XK_k }},                                          inplacerotate,         {.i = +2} }),
	&((Keychord){1, {{ MODKEY|ShiftMask, XK_j }},                                          inplacerotate,         {.i = -2} }),
	&((Keychord){1, {{ MODKEY|ShiftMask, XK_l }},                                          inplacerotate,         {.i = +1} }),
	&((Keychord){1, {{ MODKEY|ShiftMask, XK_h }},                                          inplacerotate,         {.i = -1} }),

	/* change number of windows in master frame */
	&((Keychord){1, {{ MODKEY, XK_o }},                                                    incnmaster,            {.i = +1 } }),
	&((Keychord){1, {{ MODKEY|ShiftMask, XK_o }},                                          incnmaster,            {.i = -1 } }),

	/* modify width of master region */
	&((Keychord){1, {{ MODKEY, XK_h }},                                                    setmfact,              {.f = -0.01} }),
	&((Keychord){1, {{ MODKEY, XK_l }},                                                    setmfact,              {.f = +0.01} }),


	&((Keychord){1, {{ MODKEY|ShiftMask, XK_Return }},                                     zoom,                  {0} }),

	/* switch between previous two tags */
	&((Keychord){1, {{ MODKEY, XK_Tab }},                                                  view,                  {0} }),

	/* kill focused window */
	&((Keychord){1, {{ MODKEY|ShiftMask, XK_c }},                                          killclient,            {0} }),

	/* change layouts */
	&((Keychord){2, {{ MODKEY, XK_space}, { MODKEY, XK_t }},                               setlayout,             {.v = &layouts[0]} }),
	&((Keychord){2, {{ MODKEY, XK_space}, { MODKEY, XK_f }},                               setlayout,             {.v = &layouts[1]} }),
	&((Keychord){2, {{ MODKEY, XK_space}, { MODKEY, XK_m }},                               setlayout,             {.v = &layouts[2]} }),
	&((Keychord){2, {{ MODKEY, XK_space}, { MODKEY, XK_g }},                               setlayout,             {.v = &layouts[3]} }),
	&((Keychord){2, {{ MODKEY, XK_space}, { MODKEY, XK_b }},                               setlayout,             {.v = &layouts[4]} }),
	&((Keychord){2, {{ MODKEY, XK_space }, { MODKEY, XK_space }},                          setlayout,             {0} }),

	/* floating windows */
	&((Keychord){1, {{ MODKEY|ShiftMask, XK_space }},                                      togglefloating,        {0} }),

	/* for multiple monitors */
	&((Keychord){1, {{ MODKEY, XK_comma }},                                                focusmon,              {.i = -1 } }),
	&((Keychord){1, {{ MODKEY, XK_period }},                                               focusmon,              {.i = +1 } }),
	&((Keychord){1, {{ MODKEY|ShiftMask, XK_comma }},                                      tagmon,                {.i = -1 } }),
	&((Keychord){1, {{ MODKEY|ShiftMask, XK_period }},                                     tagmon,                {.i = +1 } }),

	/* program launcher */
	&((Keychord){1, {{ MODKEY, XK_d }},                                                    spawn,                 SHCMD("dmenu_run") }),

	/* password */
	&((Keychord){1, {{ MODKEY, XK_p }},                                                    spawn,                 SHCMD("passmenu -p 'Select Password'") }),
	
	/* search for unicode characters */
	&((Keychord){1, {{ MODKEY, XK_u }},                                                    spawn,                 SHCMD("unicode-selector") }),

	/* screenlock */
	&((Keychord){1, {{ MODKEY, XK_q }},                                                    spawn,                 SHCMD("lockscreen") }),

	/* get options for reboot, shutdown, et cetera*/
	&((Keychord){1, {{ MODKEY|ShiftMask, XK_x }},                                          spawn,                 SHCMD("sys-options") }),

	/*** taking screenshots ***/
	/* fullscreen + save to folder */
	&((Keychord){1, {{ 0, XK_Print }},                                                     spawn,                 SHCMD("scrot --exec 'mv \"$f\" ~/Pictures/screenshots && notify-send \"Saved $f\"'") }),
	/* selection + save to folder */
	&((Keychord){1, {{ MODKEY, XK_Print }},                                                spawn,                 SHCMD("scrot --line style=dash,width=3,color=\"red\" --select --exec 'mv \"$f\" ~/Pictures/screenshots && notify-send \"Saved $f\"'") }),
	/* focused window + save to folder */
	&((Keychord){1, {{ControlMask, XK_Print }},                                            spawn,                 SHCMD("scrot -u --exec 'mv \"$f\" ~/Pictures/screenshots && notify-send \"Saved $f\"'")}),
	/* focused window + save to folder + copy to clipboard */
	&((Keychord){1, {{ControlMask|ShiftMask, XK_Print }},                                  spawn,                 SHCMD("scrot -u --exec 'xclip -selection clipboard -t image/png -i \"$f\" && mv \"$f\" ~/Pictures/screenshots && notify-send \"Saved $f and copied it to clipboard\"'")}),
	/* fullscreen + save to folder + copy to clipboard */
	&((Keychord){1, {{ ShiftMask, XK_Print }},                                             spawn,                 SHCMD("scrot --exec 'xclip -selection clipboard -t image/png -i \"$f\" && mv \"$f\" ~/Pictures/screenshots && notify-send \"Saved $f and copied it to clipboard\"'") }),
	/* selection + save to folder + copy to clipboard */
	&((Keychord){1, {{ MODKEY|ShiftMask, XK_Print }},                                      spawn,                 SHCMD("scrot --line style=dash,width=3,color=\"red\" --select --exec 'xclip -selection clipboard -t image/png -i \"$f\" && mv \"$f\" ~/Pictures/screenshots && notify-send \"Saved $f and copied it to clipboard\"'") }),


	/* notifications */
	&((Keychord){2, {{ MODKEY, XK_n }, { MODKEY, XK_n }},                                  spawn,                 SHCMD("dunstctl close-all") }),
	&((Keychord){2, {{ MODKEY, XK_n }, { MODKEY, XK_t }},                                  spawn,                 SHCMD("dunstctl close") }),
	&((Keychord){2, {{ MODKEY, XK_n }, { MODKEY, XK_p }},                                  spawn,                 SHCMD("dunstctl history-pop") }),

	/* brightness controls */
	&((Keychord){1, {{ MODKEY, XK_c }},                                                    spawn,                 SHCMD("clight-recalib") }),
	&((Keychord){1, {{ 0, XF86XK_MonBrightnessUp }},                                       spawn,                 SHCMD("brightnessctl set +1% ; pkill -RTMIN+19 dwmblocks") }),
	&((Keychord){1, {{ 0, XF86XK_MonBrightnessDown }},                                     spawn,                 SHCMD("brightnessctl set 1%- ; pkill -RTMIN+19 dwmblocks") }),

	/* audio/volume controls */
	&((Keychord){1, {{ 0, XF86XK_AudioMute }},                                             spawn,                 SHCMD("pamixer -t; kill -44 $(pidof dwmblocks)") }),
	&((Keychord){1, {{ 0, XF86XK_AudioRaiseVolume }},                                      spawn,                 SHCMD("pamixer --allow-boost -i 1; kill -44 $(pidof dwmblocks)") }),
	&((Keychord){1, {{ 0, XF86XK_AudioLowerVolume }},                                      spawn,                 SHCMD("pamixer --allow-boost -d 1; kill -44 $(pidof dwmblocks)") }),
	&((Keychord){1, {{ MODKEY|ControlMask, XK_space }},                                    spawn,                 SHCMD("pamixer -t; kill -44 $(pidof dwmblocks)") }),
	&((Keychord){1, {{ MODKEY|ControlMask, XK_k }},                                        spawn,                 SHCMD("pamixer --allow-boost -i 1; kill -44 $(pidof dwmblocks)") }),
	&((Keychord){1, {{ MODKEY|ControlMask, XK_j }},                                        spawn,                 SHCMD("pamixer --allow-boost -d 1; kill -44 $(pidof dwmblocks)") }),

	/* music controls */
	&((Keychord){1, {{ 0, XF86XK_AudioPrev }},                                             spawn,                 SHCMD("mpc prev") }),
	&((Keychord){1, {{ 0, XF86XK_AudioNext }},                                             spawn,                 SHCMD("mpc next") }),
	&((Keychord){1, {{ 0, XF86XK_AudioPlay }},                                             spawn,                 SHCMD("mpc toggle") }),

	/* switch between and activate tags (0 is for all) */
	&((Keychord){1, {{ MODKEY, XK_0 }},                                                    view,                  {.ui = ~0 } }),
	&((Keychord){1, {{ MODKEY|ShiftMask, XK_0 }},                                          tag,                   {.ui = ~0 } }),

	/* for switching tags */
	TAGKEYS(           XK_1,                                                               0)
	TAGKEYS(           XK_2,                                                               1)
	TAGKEYS(           XK_3,                                                               2)
	TAGKEYS(           XK_4,                                                               3)
	TAGKEYS(           XK_5,                                                               4)
	TAGKEYS(           XK_6,                                                               5)
	TAGKEYS(           XK_7,                                                               6)
	TAGKEYS(           XK_8,                                                               7)
	TAGKEYS(           XK_9,                                                               8)
};

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
	{ "borderpx",           INTEGER, &borderpx },
	{ "snap",               INTEGER, &snap },
	{ "showbar",            INTEGER, &showbar },
	{ "topbar",             INTEGER, &topbar },
	{ "nmaster",            INTEGER, &nmaster },
	{ "resizehints",        INTEGER, &resizehints },
	{ "mfact",              FLOAT,   &mfact },
	{ "color0",             STRING, &termcol0 },
	{ "color1",             STRING, &termcol1 },
	{ "color2",             STRING, &termcol2 },
	{ "color3",             STRING, &termcol3 },
	{ "color4",             STRING, &termcol4 },
	{ "color5",             STRING, &termcol5 },
	{ "color6",             STRING, &termcol6 },
	{ "color7",             STRING, &termcol7 },
	{ "color8",             STRING, &termcol8 },
	{ "color9",             STRING, &termcol9 },
	{ "color10",            STRING, &termcol10 },
	{ "color11",            STRING, &termcol11 },
	{ "color12",            STRING, &termcol12 },
	{ "color13",            STRING, &termcol13 },
	{ "color14",            STRING, &termcol14 },
	{ "color15",            STRING, &termcol15 },
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

