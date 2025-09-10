LISTADO DE MODULOS A UTILIZAR:
/* Core y memoria */
safe_str_constraint.c
safe_mem_constraint.c
memcpy_s.c, memset_s.c, memzero_s.c, memmove_s.c

/* Strings básicos */
strcpy_s.c, strcat_s.c, strcmp_s.c
strncat_s.c, strncpy_s.c, strnlen_s.c
strnterminate_s.c, strzero_s.c

/* OPCIONALES*/
strprefix_s.c      // → busca un prefijo en un substring
strstr_s.c         // → búsquedas de substring
strtok_s.c         // → divide un string por un delimitador
strspn_s.c         // → validación de caracteres válidos
strpbrk_s.c        // → búsqueda de delimitadores
strisdigit_s.c     // → validación de dígitos


MODULOS NO UTILIZADOS (ANALIZAR CASO POR CASO Y POR PROYECTO):
/* Funciones complejas/raras */
strcasecmp_s.c, strcasestr_s.c
strcmpfld_s.c, strcpyfld*.c
strisalphanumeric_s.c, strisascii_s.c
strislowercase_s.c, strismixedcase_s.c, strisuppercase_s.c
strispassword_s.c, strismixedcase_s.c
strtolowercase_s.c, strtouppercase_s.c
strremovews_s.c, strljustify_s.c
strfirstdiff_s.c, strlastdiff_s.c, etc.

/* Todas las wide char functions (wc*.c, wm*.c) */
/* Todas las variantes 16/32 bit */