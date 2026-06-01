#!/bin/bash

set -u

RED="\033[0;31m"
GREEN="\033[0;32m"
PINK='\033[0;95m'
YELLOW="\033[0;33m"
RESET="\033[0m"

VERBOSE=0
if [[ "${1:-}" == "-v" || "${1:-}" == "--verbose" ]]; then
    VERBOSE=1
fi

TESTER_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$( cd "$TESTER_DIR/.." && pwd )"
TMP_DIR="/tmp/printf_fairy_$$"

mkdir -p "$TMP_DIR"

cd "$PROJECT_DIR" || exit 1

TESTER_NAME=$(basename "$TESTER_DIR")

cleanup() {
    echo ""
    echo -e -n "🧹 Cleaning up..."
    rm -rf "$TMP_DIR"
    make fclean > /dev/null 2>&1 || true
    echo -e "\t    Done"
    echo ""
}

print_result() {
    echo ""
    if [[ $NORM_TEST_RES -eq 0 && $PROTO_TEST_RES -eq 0 && $MAKE_ERRORS -eq 0 \
        && $BASIC_BUILD_ERRORS -eq 0 && -z "$EXTERN_ERRORS" && $TEST_BUILD_ERRORS -eq 0 \
        && $BASIC_RC -eq 0 && $LEAK_RC -eq 0 && $BONUS_BUILD_ERRORS -eq 0 \
        && $BONUS_TEST_BUILD_ERRORS -eq 0 && $BONUS_BASIC_RC -eq 0 \
        && $BONUS_LEAK_RC -eq 0 ]]; then
        echo -e "${GREEN}╔══════════════════════════════╗${RESET}"
        echo -e "${GREEN}║      OH MY, YOU PASSED!      ║${RESET}"
        echo -e "${GREEN}╚══════════════════════════════╝${RESET}"
    else
        echo -e "${RED}╔══════════════════════════════╗${RESET}"
        echo -e "${RED}║     OH NO... YOU FAILED!     ║${RESET}"
        echo -e "${RED}╚══════════════════════════════╝${RESET}"
    fi
}

trap 'print_result; cleanup' EXIT INT TERM

NORM_TEST_RES=0
BONUS_VERSION=0
PROTO_TEST_RES=0
MAKE_ERRORS=0
BASIC_BUILD_ERRORS=0
EXTERN_ERRORS=""
TEST_BUILD_ERRORS=0
BASIC_RC=0
LEAK_RC=0
BONUS_BUILD_ERRORS=0
BONUS_TEST_BUILD_ERRORS=0
BONUS_BASIC_RC=0
BONUS_LEAK_RC=0

echo ""
echo -e "${PINK}╔══════════════════════════════╗${RESET}"
echo -e "${PINK}║        PRINTF-FAIRY 🧚       ║${RESET}"
echo -e "${PINK}╚══════════════════════════════╝${RESET}"
echo

echo -e -n "📝 Checking norm..."
NORM_OUTPUT=$(find . -type d -name "$TESTER_NAME" -prune -o \
    \( -name "*.c" -o -name "*.h" \) -type f -print | xargs -r norminette 2>&1)

if echo "$NORM_OUTPUT" | grep -q "Error"; then
    NORM_TEST_RES=1
    echo -e "\t  ${RED}Failed${RESET}"
    echo ""
    echo "$NORM_OUTPUT" | grep "Error"
    echo ""
else
    echo -e "\t    Done"
fi

echo -e -n "🔖 Checking version..."
if grep -qi "bonus:" Makefile; then
    BONUS_VERSION=1
    echo -e "\t   Bonus"
else
    echo -e " ${YELLOW}Mandatory${RESET}"
fi

echo -e -n "📋 Checking prototype..."
regex="int\t+ft_printf\(const char \*format, \.\.\.\)"
expected="int\tft_printf(const char *format, ...)"
if ! grep -Pq "$regex" ft_printf.h; then
    PROTO_TEST_RES=1
    echo -e "  ${RED}Failed${RESET}"
    echo ""
    echo -e "Missing or malformed prototype(s), expected:\n$expected"
    echo ""
else
    echo -e "    Done"
fi

echo -e -n "⚙️  Checking Makefile..."
MAKE_ISSUES=""
if [ ! -f "Makefile" ]; then
    echo -e "${RED}Failed${RESET}"
    echo ""
    echo "No Makefile found!"
    exit 1
fi
if ! grep -Eq '(^|\s)cc(\s|$)' Makefile; then
    ((MAKE_ERRORS++))
    MAKE_ISSUES+="Missing or incorrect compiler (should use 'cc')\n"
fi
for flag in -Wall -Wextra -Werror; do
    if ! grep -Fq -- "$flag" Makefile; then
        ((MAKE_ERRORS++))
        MAKE_ISSUES+="Missing compilation flag: $flag\n"
    fi
done
for rule in all clean fclean re; do
    if ! grep -Eq "^$rule\s*:" Makefile; then
        ((MAKE_ERRORS++))
        MAKE_ISSUES+="Missing rule: $rule\n"
    fi
done
if ! grep -Eq '^\$\(NAME\)\s*:' Makefile; then
    ((MAKE_ERRORS++))
    MAKE_ISSUES+="Missing rule: \$(NAME)\n"
fi
if [[ $BONUS_VERSION -eq 1 ]] && ! grep -Eq "^bonus:" Makefile; then
    ((MAKE_ERRORS++))
    MAKE_ISSUES+="Bonus files present but no 'bonus' rule in Makefile\n"
fi
if grep -Eq '\*\.c' Makefile || grep -Eq '\*\.o' Makefile; then
    ((MAKE_ERRORS++))
    MAKE_ISSUES+="Forbidden wildcard inclusion detected (*.c or *.o)\n"
fi
make > /dev/null 2>&1 || true
lib_file="libftprintf.a"
timestamp_before=$(stat -c %Y "$lib_file" 2>/dev/null || echo 0)
make > /dev/null 2>&1 || true
timestamp_after=$(stat -c %Y "$lib_file" 2>/dev/null || echo 0)
if [ "$timestamp_after" -ne "$timestamp_before" ]; then
    ((MAKE_ERRORS++))
    MAKE_ISSUES+="Unnecessary relink detected when running make twice\n"
fi
if [ $MAKE_ERRORS -eq 0 ]; then
    echo -e "     Done"
else
    echo -e "   ${RED}Failed${RESET}"
    echo ""
    echo -e "$MAKE_ISSUES"
    echo ""
fi

echo -e -n "📦 Building ft_printf..."
BUILD_ISSUES=""
MAKE_OUTPUT=$(make re 2>&1)
if [ $? -ne 0 ]; then
    ((BASIC_BUILD_ERRORS++))
    BUILD_ISSUES+="Command 'make re' failed:\n$MAKE_OUTPUT\n"
fi
if [ ! -f "libftprintf.a" ]; then
    ((BASIC_BUILD_ERRORS++))
    BUILD_ISSUES+="File 'libftprintf.a' was not produced.\n"
fi
if [ $BASIC_BUILD_ERRORS -eq 0 ]; then
    echo -e "    Done"
else
    echo -e "  ${RED}Failed${RESET}"
    echo ""
    echo -e "$BUILD_ISSUES"
    exit 1
fi

echo -e -n "🔍 Checking externals..."
EXTERN_ALLOWED="malloc free write memset memcpy"
EXTERN_ERRORS=""
check_obj() {
    local obj=$1
    local forbidden=""
    for ext in $(nm -u "$obj" 2>/dev/null | awk '{print $2}'); do
        [[ "$ext" == ft_* || "$ext" == __* ]] && continue
        [[ " $EXTERN_ALLOWED " =~ " $ext " ]] && continue
        forbidden="$forbidden $ext"
    done
    [[ -n "$forbidden" ]] && echo "$(basename "$obj" .o): forbidden:$forbidden"
}
for obj in *.o; do
    [[ -f "$obj" ]] || continue
    result=$(check_obj "$obj")
    [[ -n "$result" ]] && EXTERN_ERRORS="$EXTERN_ERRORS$result\n"
done
if [[ -z "$EXTERN_ERRORS" ]]; then
    echo -e "    Done"
else
    echo -e "  ${RED}Failed${RESET}"
    echo ""
    echo -e "$EXTERN_ERRORS"
    echo ""
fi

echo -e -n "🔨 Building tests..."
CFLAGS="-Wall -Wextra -Wno-format -I."
LDFLAGS="-L. -lftprintf"
WRAP_FLAGS="-Wl,--wrap=malloc"
BUILD_ISSUES=""

compile_test() {
    local src=$1
    local out=$2
    local err=$3
    cc $CFLAGS -DVERBOSE=$VERBOSE "$src" "$TESTER_NAME/utils.c" -o "$out" $WRAP_FLAGS $LDFLAGS 2> "$err" || return 1
}

compile_test "$TESTER_NAME/basic_tests.c" "$TMP_DIR/basic_test" "$TMP_DIR/basic_build.err" || { TEST_BUILD_ERRORS=1; BUILD_ISSUES+="basic_tests compilation failed:\n$(cat "$TMP_DIR/basic_build.err")\n"; }
compile_test "$TESTER_NAME/leak_tests.c" "$TMP_DIR/leak_test" "$TMP_DIR/leak_build.err" || { TEST_BUILD_ERRORS=1; BUILD_ISSUES+="leak_tests compilation failed:\n$(cat "$TMP_DIR/leak_build.err")\n"; }

if [ $TEST_BUILD_ERRORS -eq 0 ]; then
    echo -e "\t    Done"
else
    echo -e "\t  ${RED}Failed${RESET}"
    echo -e "$BUILD_ISSUES"
    exit 1
fi

echo -e -n "🧪 Running tests..."
basic_log="$TMP_DIR/basic.log"
leak_log="$TMP_DIR/leak.log"

"$TMP_DIR/basic_test" > "$basic_log" 2>&1
BASIC_RC=$?
valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all \
    --error-exitcode=42 --track-origins=yes \
    "$TMP_DIR/leak_test" >/dev/null 2>"$leak_log"
LEAK_RC=$?

if [[ $BASIC_RC -eq 0 && $LEAK_RC -eq 0 ]]; then
    echo -e "\t    Done"
else
    echo -e "\t  ${RED}Failed${RESET}"
fi
[[ $BASIC_RC -ne 0 || $VERBOSE -eq 1 ]] && echo -e "$(cat "$basic_log")"
[[ $LEAK_RC -ne 0 || $VERBOSE -eq 1 ]] && echo -e "\n════════════════════════════════════════\nVALGRIND OUTPUT (mandatory)\n════════════════════════════════════════\n$(cat "$leak_log")"

if [[ $BONUS_VERSION -eq 1 ]]; then
    echo -e -n "📦 Building bonus..."
    MAKE_OUTPUT=$(make re bonus 2>&1)
    if [ $? -ne 0 ]; then
        BONUS_BUILD_ERRORS=1
        echo -e "\t  ${RED}Failed${RESET}"
        exit 1
    else
        echo -e "\t    Done"
    fi
    echo -e -n "🔨 Building bonus tests..."
    compile_test "$TESTER_NAME/basic_tests_bonus.c" "$TMP_DIR/basic_test_bonus" "$TMP_DIR/basic_bonus_build.err" || BONUS_TEST_BUILD_ERRORS=1
    compile_test "$TESTER_NAME/leak_tests_bonus.c" "$TMP_DIR/leak_test_bonus" "$TMP_DIR/leak_bonus_build.err" || BONUS_TEST_BUILD_ERRORS=1
    if [ $BONUS_TEST_BUILD_ERRORS -eq 0 ]; then
        echo -e "  Done"
    else
        echo -e "${RED}Failed${RESET}"
        exit 1
    fi

    echo -e -n "🧪 Running bonus tests..."
    bonus_basic_log="$TMP_DIR/basic_bonus.log"
    bonus_leak_log="$TMP_DIR/leak_bonus.log"
    "$TMP_DIR/basic_test_bonus" > "$bonus_basic_log" 2>&1
    BONUS_BASIC_RC=$?
    valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all \
        --error-exitcode=42 --track-origins=yes \
        "$TMP_DIR/leak_test_bonus" >/dev/null 2>"$bonus_leak_log"
    BONUS_LEAK_RC=$?
    
    if [[ $BONUS_BASIC_RC -eq 0 && $BONUS_LEAK_RC -eq 0 ]]; then
        echo -e "   Done"
    else
        echo -e " ${RED}Failed${RESET}"
    fi
    [[ $BONUS_BASIC_RC -ne 0 || $VERBOSE -eq 1 ]] && echo -e "$(cat "$bonus_basic_log")"
    [[ $BONUS_LEAK_RC -ne 0 || $VERBOSE -eq 1 ]] && echo -e "\n════════════════════════════════════════\nVALGRIND OUTPUT (bonus)\n════════════════════════════════════════\n$(cat "$bonus_leak_log")"
fi

exit 0

