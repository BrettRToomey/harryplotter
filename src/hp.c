#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define METATABLE_PLOT ("HPPlot")

// Plot
struct Plot {
    const char *title;
    const char *xaxis;
    const char *yaxis;

    const char **tempFiles;
    int tempFileCount;
    int tempFileCap;
};

const char *getTempName(struct Plot *plot) {
    char template[] = "/tmp/hpXXXXXX";
    mktemp(template);
    const char *tmp = strdup(template);
    plot->tempFiles[plot->tempFileCount++] = tmp;
    if (plot->tempFileCount >= plot->tempFileCap) {
        plot->tempFileCap *= 2;
        plot->tempFiles = realloc(plot->tempFiles, plot->tempFileCap * sizeof(const char *));
    }

    return tmp;
}

int newPlot(lua_State *L) {
    struct Plot *plot = lua_newuserdata(L, sizeof(struct Plot));
    luaL_setmetatable(L, METATABLE_PLOT);

    memset(plot, 0, sizeof(struct Plot));

    if (lua_gettop(L) && lua_istable(L, 1)) {
        lua_getfield(L, 1, "title");
        const char *title = lua_tostring(L, -1);
        if (title) {
			plot->title = strdup(title);
        }
        lua_pop(L, 1);

        lua_getfield(L, 1, "xaxis");
        const char *xaxis = lua_tostring(L, -1);
        if (xaxis) {
			plot->xaxis = strdup(xaxis);
        }
        lua_pop(L, 1);

        lua_getfield(L, 1, "yaxis");
        const char *yaxis = lua_tostring(L, -1);
        if (yaxis) {
			plot->yaxis = strdup(yaxis);
        }
        lua_pop(L, 1);
    }

    plot->tempFiles = malloc(16 * sizeof(const char *));
    plot->tempFileCap = 16;

    return 1;
}

int freePlot(lua_State *L) {
    struct Plot *plot = luaL_checkudata(L, 1, METATABLE_PLOT);
    for (int i = 0; i < plot->tempFileCount; i++) {
        free((void *)plot->tempFiles[i]);
    }

    return 0;
}

int plotLine(lua_State *L) {
    struct Plot *plot = luaL_checkudata(L, 1, METATABLE_PLOT);
    luaL_checktype(L, 2, LUA_TTABLE);
    luaL_checktype(L, 3, LUA_TTABLE);
    if (!lua_isnoneornil(L, 4)) {
        luaL_checktype(L, 4, LUA_TTABLE);
    }

    size_t lhsCount = lua_rawlen(L, 2);
    size_t rhsCount = lua_rawlen(L, 3);
    size_t count = lhsCount <= rhsCount ? lhsCount : rhsCount;

    const char *tmpName = getTempName(plot);
    FILE *f = fopen(tmpName, "w");
    printf("Temp: %s\n", tmpName);

	for (size_t i = 1; i <= count; i++) {
		lua_geti(L, 2, i);
		lua_geti(L, 3, i);

		double x = lua_tonumber(L, -2);
		double y = lua_tonumber(L, -1);

		lua_pop(L, 2);

		fprintf(f, "%f\t%f\n", x, y);
	}

	fclose(f);
    return 0;
}

int plotShow(lua_State *L) {
    struct Plot *plot = luaL_checkudata(L, 1, METATABLE_PLOT);

    char plotPath[] = "/tmp/harryplotterXXXXXX";
    mktemp(plotPath);
    printf("plot: %s\n", plotPath);

	FILE *f = fopen(plotPath, "w");

	// Options
	fprintf(f, "set terminal qt font 'Helvetica,12'\n");
	fprintf(f, "set key font \",12\"\n");
	fprintf(f, "unset for [i=1:32] linetype i\n");
	fprintf(f, "set linetype 1 lw 2 lc rgb \"red\" pt 4\n");
	fprintf(f, "set linetype 2 lw 2 lc rgb \"purple\" pt 4\n");
	fprintf(f, "set linetype cycle 2\n");
	fprintf(f, "set title \"%s\"\n", plot->title);

	if (plot->xaxis) {
		fprintf(f, "set xlabel \"%s\"\n", plot->xaxis);
	}

	if (plot->yaxis) {
		fprintf(f, "set ylabel \"%s\"\n", plot->yaxis);
	}

	{ // Plot
    	fprintf(f, "plot ");

        for (int i = 0; i < plot->tempFileCount; i++) {
            if (i != 0) fprintf(f, ",");
            fprintf(f, "\"%s\" with lines", plot->tempFiles[i]);
        }

    	fprintf(f, "\n");
	}

    fclose(f);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "gnuplot -p -c %s", plotPath);
	system(cmd);

    return 0;
}

static const struct luaL_Reg plotMethods[] = {
    { "__gc", freePlot },
    { "line", plotLine },
    { "show", plotShow },
    { NULL, NULL },
};

// Harry Plotter
static const struct luaL_Reg harryplotterLib[] = {
    { "new", newPlot },
    { NULL, NULL },
};

int luaopen_harryplotter(lua_State *L) {
    {
        luaL_newmetatable(L, METATABLE_PLOT);
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
        luaL_setfuncs(L, plotMethods, 0);
        lua_pop(L, 1);
    }

    luaL_newlib(L, harryplotterLib);
    return 1;
}
