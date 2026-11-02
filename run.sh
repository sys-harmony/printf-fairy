#!/bin/bash

set -u

RED="\033[0;31m"
GREEN="\033[0;32m"
PINK='\033[0;95m'
YELLOW="\033[0;33m"
RESET="\033[0m"
BAR="════════════════════════════════════════"

VERBOSE=0
if [[ "${1:-}" == "-v" || "${1:-}" == "--verbose" ]]; then
    VERBOSE=1
fi

TESTER_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$( cd "$TESTER_DIR/.." && pwd )"
TMP_DIR=$(mktemp -d /tmp/printf_fairy.XXXXXX) || exit 1

cd "$PROJECT_DIR" || exit 1

TESTER_NAME=$(basename "$TESTER_DIR")

FAILED=0
COMPLETED=0

# ---- alignment helpers ------------------------------------------------------
COL=33
ok()   { printf "\033[${COL}G  Done\n"; }
fail() { FAILED=1; printf "\033[${COL}G${RED}Failed${RESET}\n"; }

cleanup() {
    echo ""
    printf "🧹 Cleaning up..."
    rm -rf "$TMP_DIR"
    make fclean > /dev/null 2>&1 || true
    ok
    echo ""
}

print_result() {
    # Interrupted before the end with nothing failed yet: no verdict.
    [[ $FAILED -eq 0 && $COMPLETED -eq 0 ]] && return
    echo ""
    if [[ $FAILED -eq 0 ]]; then
        echo -e "${GREEN}╔════════════════════════════════════╗${RESET}"
        echo -e "${GREEN}║         OH MY, YOU PASSED!         ║${RESET}"
        echo -e "${GREEN}╚════════════════════════════════════╝${RESET}"
    else
        echo -e "${RED}╔════════════════════════════════════╗${RESET}"
        echo -e "${RED}║        OH NO... YOU FAILED!        ║${RESET}"
        echo -e "${RED}╚════════════════════════════════════╝${RESET}"
    fi
}

trap 'print_result; cleanup' EXIT
trap 'exit 130' INT TERM

echo ""
echo -e "${PINK}╔════════════════════════════════════╗${RESET}"
echo -e "${PINK}║           PRINTF-FAIRY 🧚          ║${RESET}"
echo -e "${PINK}╚════════════════════════════════════╝${RESET}"
echo

printf "📝 Checking norm..."
NORM_OUTPUT=$(find . -type d -name "$TESTER_NAME" -prune -o \
    \( -name "*.c" -o -name "*.h" \) -type f -print | xargs -r norminette 2>&1)
case $? in
    0)   ok ;;
    127) printf "\033[32G${YELLOW}Skipped${RESET}\n\nnorminette not found\n\n" ;;
    *)   fail
         echo ""
         echo "$NORM_OUTPUT" | grep -Ev 'OK!$|^Setting locale'
         echo "" ;;
esac

printf "🔖 Checking version..."
if grep -Eqs '^bonus[[:space:]]*:' Makefile; then
    BONUS_VERSION=1
    printf "\033[34GBonus\n"
else
    BONUS_VERSION=0
    printf "\033[30G${YELLOW}Mandatory${RESET}\n"
fi

printf "📋 Checking prototype..."
PROTO_REGEX='(^|[^_[:alnum:]])int[[:space:]]+ft_printf[[:space:]]*\([[:space:]]*'
PROTO_REGEX+='const[[:space:]]+char[[:space:]]*\*[[:space:]]*([A-Za-z_][A-Za-z0-9_]*)?'
PROTO_REGEX+='[[:space:]]*,[[:space:]]*\.\.\.[[:space:]]*\)'
# -z: a prototype split over several lines still matches.
PROTO_HITS=$(find . -type d -name "$TESTER_NAME" -prune -o \
    -name "*.h" -type f -print \
    | xargs -r grep -lzP "$PROTO_REGEX" 2>/dev/null)
if [ -z "$PROTO_HITS" ]; then
    fail
    echo ""
    echo -e "Missing or malformed prototype, expected:\nint\tft_printf(const char *, ...)"
    echo ""
else
    ok
fi

# Succeeds if running `make <args>` again rewrites libftprintf.a. %y has
# nanosecond precision: two quick builds often happen in the same second.
relinks() {
    local before
    before=$(stat -c %y libftprintf.a 2>/dev/null)
    make "$@" > /dev/null 2>&1
    [[ "$(stat -c %y libftprintf.a 2>/dev/null)" != "$before" ]]
}

printf "⚙️  Checking Makefile..."
if [ ! -f "Makefile" ]; then
    fail
    echo ""
    echo "No Makefile found!"
    exit 1
fi
MAKE_ISSUES=""
if ! grep -Eq '(^|\s)cc(\s|$)' Makefile; then
    MAKE_ISSUES+="Missing or incorrect compiler (should use 'cc')\n"
fi
for flag in -Wall -Wextra -Werror; do
    if ! grep -Fq -- "$flag" Makefile; then
        MAKE_ISSUES+="Missing compilation flag: $flag\n"
    fi
done
for rule in all clean fclean re; do
    if ! grep -Eq "^$rule\s*:" Makefile; then
        MAKE_ISSUES+="Missing rule: $rule\n"
    fi
done
if ! grep -Eq '^\$[({]NAME[)}]\s*:' Makefile; then
    MAKE_ISSUES+="Missing rule: \$(NAME)\n"
fi
# Only variable definitions: `rm -f *.o` in a recipe is fine.
if grep -Eq '\$\(wildcard|^[^#[:space:]][^#]*=[^#]*\*\.[co]\b' Makefile; then
    MAKE_ISSUES+="Forbidden wildcard inclusion detected (*.c or *.o)\n"
fi
make > /dev/null 2>&1
if relinks; then
    MAKE_ISSUES+="Unnecessary relink detected when running make twice\n"
fi
if [[ $BONUS_VERSION -eq 1 ]]; then
    make bonus > /dev/null 2>&1
    if relinks bonus; then
        MAKE_ISSUES+="Unnecessary relink detected when running make bonus twice\n"
    fi
fi
if [[ -z "$MAKE_ISSUES" ]]; then
    ok
else
    fail
    echo ""
    echo -e "$MAKE_ISSUES"
fi

# memset/memcpy may be emitted by the compiler (clang) for struct copies or
# initialisation, __stack_chk_fail by gcc's stack protector. va_* are builtins.
EXTERN_ALLOWED='malloc|free|write|memset|memcpy|__stack_chk_fail'

# Reads the archive itself, so objects built in subfolders are checked too.
check_externals() {
    local forbidden
    forbidden=$(comm -23 \
        <(nm -u libftprintf.a 2>/dev/null | awk '$1 == "U" {print $2}' | sort -u) \
        <(nm --defined-only libftprintf.a 2>/dev/null | awk 'NF == 3 {print $3}' | sort -u) \
        | grep -vxE "$EXTERN_ALLOWED")
    if [[ -z "$forbidden" ]]; then
        ok
        return
    fi
    fail
    echo ""
    echo "Forbidden function(s):"
    nm -A -u libftprintf.a 2>/dev/null | grep -wF "$forbidden" \
        | awk '{sub(/^libftprintf\.a:/, "", $1); print "  " $1, $NF}'
    echo ""
}

CFLAGS="-Wall -Wextra -Wno-format -I."
for h in $PROTO_HITS; do
    dir=$(dirname "$h")
    case " $CFLAGS " in
        *" -I$dir "*) ;;
        *) CFLAGS="$CFLAGS -I$dir" ;;
    esac
done
PROTO_HEADER=$(echo "$PROTO_HITS" | head -1)
[[ -n "$PROTO_HEADER" ]] && CFLAGS="$CFLAGS -include $PROTO_HEADER"
LDFLAGS="-L. -lftprintf -Wl,--wrap=malloc"

compile_test() {
    cc $CFLAGS -DVERBOSE=$VERBOSE "$TESTER_NAME/$1.c" "$TESTER_NAME/utils.c" \
        -o "$TMP_DIR/$1" $LDFLAGS 2> "$TMP_DIR/$1.err"
}

# run_phase <step prefix> <make args> <tests...>
# Builds the library, checks its symbols, then builds and runs each test
# (leak_* ones under valgrind). Stops the run if something does not build.
run_phase() {
    local prefix=$1 make_args=$2 out t issues="" failed=" "
    shift 2

    printf "📦 Building %sft_printf..." "$prefix"
    out=$(make $make_args 2>&1)
    if [[ $? -ne 0 || ! -f libftprintf.a ]]; then
        fail
        printf "\n%s\n" "$out"
        [[ -f libftprintf.a ]] || echo "File 'libftprintf.a' was not produced."
        exit 1
    fi
    ok

    printf "🔍 Checking %sexternals..." "$prefix"
    check_externals

    printf "🔨 Building %stests..." "$prefix"
    for t in "$@"; do
        compile_test "$t" \
            || issues+="$t compilation failed:"$'\n'"$(cat "$TMP_DIR/$t.err")"$'\n'
    done
    if [[ -n "$issues" ]]; then
        fail
        printf "\n%s\n" "$issues"
        exit 1
    fi
    ok

    printf "🧪 Running %stests..." "$prefix"
    for t in "$@"; do
        if [[ $t == leak_* ]]; then
            timeout 60 valgrind --leak-check=full --show-leak-kinds=all \
                --errors-for-leak-kinds=all --error-exitcode=42 --track-origins=yes \
                "$TMP_DIR/$t" > /dev/null 2> "$TMP_DIR/$t.log"
        else
            "$TMP_DIR/$t" > "$TMP_DIR/$t.log" 2>&1
        fi || failed+="$t "
    done
    if [[ $failed == " " ]]; then
        ok
    else
        fail
    fi
    for t in "$@"; do
        [[ $VERBOSE -eq 1 || $failed == *" $t "* ]] || continue
        [[ $t == leak_* ]] && printf "\n%s\nVALGRIND OUTPUT (%s)\n%s\n" "$BAR" "$t" "$BAR"
        cat "$TMP_DIR/$t.log"
    done
}

run_phase "" "re" basic_tests leak_tests
if [[ $BONUS_VERSION -eq 1 ]]; then
    # Mandatory tests run again: the bonus build may reimplement conversions.
    run_phase "bonus " "re bonus" basic_tests basic_tests_bonus leak_tests leak_tests_bonus
fi

COMPLETED=1
exit "$FAILED"
