/*
 * tkvdb
 *
 * Copyright (c) 2018, Vladimir Misyurov
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static const char *funcs[] = {
	"put",
	"get",
	"cursor_push",
	"cursor_pop",
	"cursor_append",
	"node_alloc",
	"node_new",
	"clone_subnodes",
	"seek",
	"first",
	"last",
	"next",
	"prev",
	"smallest",
	"biggest",
	"cursor_append_sym",
	"cursor_load_root",
	"node_read",
	"node_free",
	"memnode",
	"tr_reset",
	"tr_free",
	"rollback",
	"node_to_buf",
	"node_calc_disksize",
	"do_commit",
	"commit",
	"do_del",
	"del",
	NULL
};

/* functions with triggers */
static const char *trigfuncs[] = {
	"put",
	"get",
	NULL
};

static const char *incs[] = {
	"impl/memnode.h",
	"impl/node.c",
	"impl/put.c",
	"impl/get.c",
	"impl/cursor.c",
	"impl/tr.c",
	"impl/del.c"
};

static char *
str2upper(const char *lower)
{
	char *upper;
	size_t i;

	upper = strdup(lower);
	for (i=0; i<strlen(upper); i++) {
		upper[i] = toupper(upper[i]);
	}

	return upper;
}

static void
print_block(const char *name, int dbfile)
{
	size_t i;
	const char *func;
	char *func_upper;

	printf("#define TKVDB_MEMNODE_TYPE tkvdb_memnode_%s%s\n",
		name, dbfile ? "": "_nodb");
	printf("#define TKVDB_MEMNODE_TYPE_COMMON tkvdb_memnode_%s%s_common\n",
		name, dbfile ? "": "_nodb");
	printf("#define TKVDB_MEMNODE_TYPE_LEAF tkvdb_memnode_%s%s_leaf\n",
		name, dbfile ? "": "_nodb");
	for (i=0; funcs[i]; i++) {
		func = funcs[i];
		func_upper = str2upper(func);
		printf("#define TKVDB_IMPL_%s tkvdb_%s_%s%s\n",
			func_upper, func, name, dbfile ? "": "_nodb");
		free(func_upper);
	}

	if (strcmp(name, "alignval") == 0) {
		printf("\n#define TKVDB_PARAMS_ALIGN_VAL\n\n");
	}
	if (!dbfile) {
		printf("\n#define TKVDB_PARAMS_NODBFILE\n\n");
	}

	for (i=0; incs[i]; i++) {
		printf("#include \"%s\"\n", incs[i]);
	}

	/* triggers */
	printf("\n");
	printf("#define TKVDB_TRIGGER\n");

	for (i=0; trigfuncs[i]; i++) {
		func = trigfuncs[i];
		func_upper = str2upper(func);
		printf("#undef TKVDB_IMPL_%s\n", func_upper);
		printf("#define TKVDB_IMPL_%s tkvdb_%s_%s%sx\n",
			func_upper, func, name, dbfile ? "": "_nodb");
		printf("#include \"impl/%s.c\"\n", func);
		free(func_upper);
	}
	printf("#undef TKVDB_TRIGGER\n");

	/* undefine all */
	printf("\n");
	for (i=0; funcs[i]; i++) {
		func = funcs[i];
		func_upper = str2upper(func);
		printf("#undef TKVDB_IMPL_%s\n", func_upper);
		free(func_upper);
	}

	if (strcmp(name, "alignval") == 0) {
		printf("\n#undef TKVDB_PARAMS_ALIGN_VAL\n\n");
	}
	if (!dbfile) {
		printf("\n#undef TKVDB_PARAMS_NODBFILE\n\n");
	}
	printf("#undef TKVDB_SUBNODE_NEXT\n");
	printf("#undef TKVDB_SUBNODE_SEARCH\n\n");

	printf("#undef TKVDB_MEMNODE_TYPE\n");
	printf("#undef TKVDB_MEMNODE_TYPE_COMMON\n");
	printf("#undef TKVDB_MEMNODE_TYPE_LEAF\n\n\n");
}

int
main(int argc, char *argv[])
{
	(void)argc;
	printf("/* GENERATED BY %s, PLEASE DON'T EDIT THIS FILE DIRECTLY */\n",
		argv[0]);
	print_block("alignval", 1);
	print_block("generic", 1);

	/* RAM-only database, without underlying file */
	print_block("alignval", 0);
	print_block("generic", 0);

	return EXIT_SUCCESS;
}

