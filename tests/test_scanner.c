#include "error.h"
#include "scanner.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TMP_FILE "test_input.tmp"

static int testu_celkem = 0;
static int testu_chyb = 0;

#define CHECK(cond)                                                                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        testu_celkem++;                                                                                                \
        if (!(cond))                                                                                                   \
        {                                                                                                              \
            testu_chyb++;                                                                                              \
            printf("  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, #cond);                                                 \
        }                                                                                                              \
    } while (0)

static int zacatek_radku = 1;
static int hloubka_zavorek = 0;

void scanner_reset()
{
    zacatek_radku = 1;
    hloubka_zavorek = 0;
}

static void set_input(const char *text)
{
    FILE *f = fopen(TMP_FILE, "w");
    if (f == NULL)
    {
        perror("fopen");
        exit(99);
    }
    fputs(text, f);
    fclose(f);

    if (freopen(TMP_FILE, "r", stdin) == NULL)
    {
        perror("freopen");
        exit(99);
    }
    scanner_reset();
}

static void expect_token(TokenType type)
{
    Token t;
    int rc = get_next_token(&t);
    CHECK(rc == ERR_OK);
    CHECK(t.type == type);
    if (t.string != NULL)
        free(t.string);
}

static void expect_error(int err)
{
    Token t;
    int rc = get_next_token(&t);
    CHECK(rc == err);
}

static void test_klicova_slova()
{
    printf("test_klicova_slova\n");
    set_input("while if else var val def return do then import Int Double String Unit");

    expect_token(TOKEN_WHILE);
    expect_token(TOKEN_IF);
    expect_token(TOKEN_ELSE);
    expect_token(TOKEN_VAR);
    expect_token(TOKEN_VAL);
    expect_token(TOKEN_DEF);
    expect_token(TOKEN_RETURN);
    expect_token(TOKEN_DO);
    expect_token(TOKEN_THEN);
    expect_token(TOKEN_IMPORT);
    expect_token(TOKEN_INT);
    expect_token(TOKEN_DOUBLE);
    expect_token(TOKEN_STRING);
    expect_token(TOKEN_UNIT);
    expect_token(TOKEN_EOF);
}

static void test_identifikator()
{
    printf("test_identifikator\n");
    set_input("promenna _abc x1");

    Token t;

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_ID);
    CHECK(strcmp(t.string, "promenna") == 0);
    free(t.string);

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_ID);
    CHECK(strcmp(t.string, "_abc") == 0);
    free(t.string);

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_ID);
    CHECK(strcmp(t.string, "x1") == 0);
    free(t.string);
}

static void test_cela_cisla()
{
    printf("test_cela_cisla\n");
    set_input("42 0 1_000");

    Token t;

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_INT_HOD);
    CHECK(t.int_val == 42);

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_INT_HOD);
    CHECK(t.int_val == 0);

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_INT_HOD);
    CHECK(t.int_val == 1000);
}

static void test_desetinna_cisla()
{
    printf("test_desetinna_cisla\n");
    set_input("3.14 2e3 .5");

    Token t;

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_DOUBLE_HOD);
    CHECK(t.double_val > 3.139 && t.double_val < 3.141);

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_DOUBLE_HOD);
    CHECK(t.double_val == 2000.0);

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_DOUBLE_HOD);
    CHECK(t.double_val == 0.5);
}

static void test_retezce()
{
    printf("test_retezce\n");
    set_input("\"ahoj\" \"a\\nb\" \"\"");

    Token t;

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_STRING_HOD);
    CHECK(strcmp(t.string, "ahoj") == 0);
    free(t.string);

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_STRING_HOD);
    CHECK(strcmp(t.string, "a\nb") == 0);
    free(t.string);

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_STRING_HOD);
    CHECK(strcmp(t.string, "") == 0);
    free(t.string);
}

static void test_operatory()
{
    printf("test_operatory\n");
    set_input("= == < <= > >= != + - * ( ) : , ; / . @");

    expect_token(TOKEN_ROVNO);
    expect_token(TOKEN_STEJNO);
    expect_token(TOKEN_MENSI);
    expect_token(TOKEN_MENSI_ROVNO);
    expect_token(TOKEN_VETSI);
    expect_token(TOKEN_VETSI_ROVNO);
    expect_token(TOKEN_NE_ROVNO);
    expect_token(TOKEN_PLUS);
    expect_token(TOKEN_MINUS);
    expect_token(TOKEN_NAS);
    expect_token(TOKEN_LZAVORKA);
    expect_token(TOKEN_PZAVORKA);
    expect_token(TOKEN_DVOJTEC);
    expect_token(TOKEN_CARKA);
    expect_token(TOKEN_STREDNIK);
    expect_token(TOKEN_DEL);
    expect_token(TOKEN_TECKA);
    expect_token(TOKEN_ZAVINAC);
}

static void test_komentare()
{
    printf("test_komentare\n");

    set_input("// komentar\nvar");
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_VAR);

    set_input("/* x */ var");
    expect_token(TOKEN_VAR);

    set_input("/* a /* b */ c */ var");
    expect_token(TOKEN_VAR);
}

static void test_lexikalni_chyby()
{
    printf("test_lexikalni_chyby\n");

    set_input("\"neukonceny");
    expect_error(ERR_LEX);

    set_input("/* neukonceny komentar");
    expect_error(ERR_LEX);

    set_input("01");
    expect_error(ERR_LEX);

    set_input("1e");
    expect_error(ERR_LEX);

    set_input("!");
    expect_error(ERR_LEX);

    set_input("\"\\q\"");
    expect_error(ERR_LEX);
}

static void test_eol_a_eof()
{
    printf("test_eol_a_eof\n");
    set_input("var\nval");

    expect_token(TOKEN_VAR);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_VAL);
    expect_token(TOKEN_EOF);
}

static void test_specialni_cisla()
{
    printf("test_specialni_cisla\n");
    set_input("1d 1.4d 2e+6 2e-6 1.5e3");

    Token t;

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_DOUBLE_HOD);
    CHECK(t.double_val == 1);

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_DOUBLE_HOD);
    CHECK(t.double_val == 1.4);

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_DOUBLE_HOD);
    CHECK(t.double_val == 2e+6);

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_DOUBLE_HOD);
    CHECK(t.double_val == 2e-6);

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_DOUBLE_HOD);
    CHECK(t.double_val == 1.5e3);
}

static void test_id()
{
    printf("test_id\n");
    set_input("whilex returnn Int2 Unitt");

    expect_token(TOKEN_ID);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_ID);
}

static void test_cisla_chyby()
{
    printf("test_cisla_chyby\n");

    const char *spatne[] = {"1..2", "1.", "1.5.2", "1ee3", "1e+", "1_", "1__0", "0_1", "1dd", "1d5"};
    for (size_t i = 0; i < sizeof(spatne) / sizeof(spatne[0]); i++)
    {
        set_input(spatne[i]);
        expect_error(ERR_LEX);
    }
}

static void test_viceradkove_retezce()
{
    printf("test_viceradkove_retezce\n");
    set_input("\"\"\"ab\ncd\"\"\" \"\"\"\"\"\"");

    Token t;

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_STRING_HOD);
    CHECK(strcmp(t.string, "ab\ncd") == 0);
    free(t.string);

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_STRING_HOD);
    CHECK(strcmp(t.string, "") == 0);
    free(t.string);
}

static void test_bez_mezer()
{
    printf("test_bez_mezer\n");
    set_input("a=b+1 x<=5 f(1,2) a!=b 1d+2 3.5*2");

    expect_token(TOKEN_ID);
    expect_token(TOKEN_ROVNO);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_PLUS);
    expect_token(TOKEN_INT_HOD);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_MENSI_ROVNO);
    expect_token(TOKEN_INT_HOD);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_LZAVORKA);
    expect_token(TOKEN_INT_HOD);
    expect_token(TOKEN_CARKA);
    expect_token(TOKEN_INT_HOD);
    expect_token(TOKEN_PZAVORKA);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_NE_ROVNO);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_DOUBLE_HOD);
    expect_token(TOKEN_PLUS);
    expect_token(TOKEN_INT_HOD);
    expect_token(TOKEN_DOUBLE_HOD);
    expect_token(TOKEN_NAS);
    expect_token(TOKEN_INT_HOD);
}

static void test_prazdny_vstup()
{
    printf("test_prazdny_vstup\n");

    set_input("");
    expect_token(TOKEN_EOF);

    set_input("   ");
    expect_token(TOKEN_EOF);
}

static void test_escape()
{
    printf("test_escape\n");
    set_input("\"a\\\\b\\\"c\\rd\\te\"");

    Token t;

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_STRING_HOD);
    CHECK(strcmp(t.string, "a\\b\"c\rd\te") == 0);
    free(t.string);
}

static void test_retezce_chyby()
{
    printf("test_retezce_chyby\n");

    const char *spatne[] = {"\"abc\ndef\"", "\"abc\\", "\"\"\"neukonceny"};
    for (size_t i = 0; i < sizeof(spatne) / sizeof(spatne[0]); i++)
    {
        set_input(spatne[i]);
        expect_error(ERR_LEX);
    }
}

static void test_dlouhy_identifikator()
{
    printf("test_dlouhy_identifikator\n");

    char buf[400];
    memset(buf, 'a', 399);
    buf[399] = '\0';

    set_input(buf);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOF);
}

static void test_d_okraje()
{
    printf("test_d_okraje\n");

    set_input("1d");
    expect_token(TOKEN_DOUBLE_HOD);
    expect_token(TOKEN_EOF);

    set_input("(1d)");
    expect_token(TOKEN_LZAVORKA);
    expect_token(TOKEN_DOUBLE_HOD);
    expect_token(TOKEN_PZAVORKA);

    set_input("1D 1_000.5");
    expect_token(TOKEN_DOUBLE_HOD);
    expect_token(TOKEN_DOUBLE_HOD);
}

static void test_program()
{
    printf("test_program\n");
    set_input("import ifj26.*\n"
              "@main def main(): Unit =\n"
              "  var x: Int = 1_000\n"
              "  while x > 0 do\n"
              "    x = x - 1\n");

    expect_token(TOKEN_IMPORT);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_TECKA);
    expect_token(TOKEN_NAS);
    expect_token(TOKEN_EOL);

    expect_token(TOKEN_ZAVINAC);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_DEF);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_LZAVORKA);
    expect_token(TOKEN_PZAVORKA);
    expect_token(TOKEN_DVOJTEC);
    expect_token(TOKEN_UNIT);
    expect_token(TOKEN_ROVNO);
    expect_token(TOKEN_EOL);

    expect_token(TOKEN_VAR);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_DVOJTEC);
    expect_token(TOKEN_INT);
    expect_token(TOKEN_ROVNO);
    expect_token(TOKEN_INT_HOD);
    expect_token(TOKEN_EOL);

    expect_token(TOKEN_WHILE);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_VETSI);
    expect_token(TOKEN_INT_HOD);
    expect_token(TOKEN_DO);
    expect_token(TOKEN_EOL);

    expect_token(TOKEN_ID);
    expect_token(TOKEN_ROVNO);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_MINUS);
    expect_token(TOKEN_INT_HOD);
    expect_token(TOKEN_EOL);

    expect_token(TOKEN_EOF);
}

static void test_odsazeni_hodnoty()
{
    printf("test_odsazeni_hodnoty\n");
    set_input("a\n  b\n    c\nd\n");

    Token t;

    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_ODSAZENI);
    CHECK(t.int_val == 2);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_ODSAZENI);
    CHECK(t.int_val == 4);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);

    CHECK(get_next_token(&t) == ERR_OK);
    CHECK(t.type == TOKEN_ODSAZENI);
    CHECK(t.int_val == 0);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);

    expect_token(TOKEN_EOF);
}

static void test_odsazeni_else()
{
    printf("test_odsazeni_else\n");
    set_input("if x then\n  a\nelse\n  b\n");

    expect_token(TOKEN_IF);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_THEN);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_ODSAZENI);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_ODSAZENI);
    expect_token(TOKEN_ELSE);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_ODSAZENI);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_EOF);
}

static void test_prazdne_radky()
{
    printf("test_prazdne_radky\n");

    set_input("a\n\n   \n  b\n");
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_ODSAZENI);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_EOF);

    set_input("\n\n  \na\n");
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_EOF);
}

static void test_radky_s_komentarem()
{
    printf("test_radky_s_komentarem\n");

    set_input("a\n      // komentar\n  b\n");
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_ODSAZENI);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_EOF);

    set_input("a // komentar\nb\n");
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_ODSAZENI);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_EOF);
}

static void test_tabulator()
{
    printf("test_tabulator\n");

    set_input("\tvar\n");
    expect_error(ERR_LEX);

    set_input("a\n\tb\n");
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);
    expect_error(ERR_LEX);

    /* řádek jen z tabulátoru je prázdný (moje interpretace zadání) */
    set_input("a\n\t\nb\n");
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_ODSAZENI);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_EOF);
}

static void test_zavorky_novy_radek()
{
    printf("test_zavorky_novy_radek\n");

    /* uvnitř závorek žádné EOL ani ODSAZENI */
    set_input("f(1,\n    2)\n");
    expect_token(TOKEN_ID);
    expect_token(TOKEN_LZAVORKA);
    expect_token(TOKEN_INT_HOD);
    expect_token(TOKEN_CARKA);
    expect_token(TOKEN_INT_HOD);
    expect_token(TOKEN_PZAVORKA);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_EOF);

    set_input("f(g(1,\n      2),\n    3)\n");
    expect_token(TOKEN_ID);
    expect_token(TOKEN_LZAVORKA);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_LZAVORKA);
    expect_token(TOKEN_INT_HOD);
    expect_token(TOKEN_CARKA);
    expect_token(TOKEN_INT_HOD);
    expect_token(TOKEN_PZAVORKA);
    expect_token(TOKEN_CARKA);
    expect_token(TOKEN_INT_HOD);
    expect_token(TOKEN_PZAVORKA);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_EOF);
}

static void test_zavorky_spatne_odsazeni()
{
    printf("test_zavorky_spatne_odsazeni\n");

    set_input("f(1,\n2)\n");
    expect_token(TOKEN_ID);
    expect_token(TOKEN_LZAVORKA);
    expect_token(TOKEN_INT_HOD);
    expect_token(TOKEN_CARKA);
    expect_error(ERR_LEX);
}

static void test_blokovy_komentar_novy_radek()
{
    printf("test_blokovy_komentar_novy_radek\n");

    set_input("a /* x */ b\n");
    expect_token(TOKEN_ID);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_EOF);

    set_input("a /* x\ny */\nb\n");
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_ODSAZENI);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_EOF);

    set_input("a /* x\ny */ b\n");
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);
    expect_error(ERR_LEX);
}

static void test_konec_bez_noveho_radku()
{
    printf("test_konec_bez_noveho_radku\n");
    set_input("a\n  b");

    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOL);
    expect_token(TOKEN_ODSAZENI);
    expect_token(TOKEN_ID);
    expect_token(TOKEN_EOF);
}

static void test_podtrzitko()
{
    printf("test_podtrzitko\n");

    set_input("_");
    expect_error(ERR_LEX);

    set_input("__ _1");
    expect_token(TOKEN_ID);
    expect_token(TOKEN_ID);
}

int main()
{
    test_klicova_slova();
    test_identifikator();
    test_cela_cisla();
    test_desetinna_cisla();
    test_retezce();
    test_operatory();
    test_komentare();
    test_lexikalni_chyby();
    test_eol_a_eof();
    test_specialni_cisla();
    test_id();
    test_cisla_chyby();
    test_viceradkove_retezce();
    test_bez_mezer();
    test_prazdny_vstup();
    test_escape();
    test_retezce_chyby();
    test_dlouhy_identifikator();
    test_d_okraje();
    test_program();
    test_odsazeni_hodnoty();
    test_odsazeni_else();
    test_prazdne_radky();
    test_radky_s_komentarem();
    test_tabulator();
    test_zavorky_novy_radek();
    test_zavorky_spatne_odsazeni();
    test_blokovy_komentar_novy_radek();
    test_konec_bez_noveho_radku();
    test_podtrzitko();

    remove(TMP_FILE);

    printf("\nHotovo: %d kontrol, %d chyb\n", testu_celkem, testu_chyb);
    return testu_chyb == 0 ? 0 : 1;
}