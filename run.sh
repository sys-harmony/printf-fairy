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

cd "$PROJECT_DIR" || exit 1

echo -e "${PINK}╔═══════════════════════════╗${RESET}"
echo -e "${PINK}║      PRINTF-FAIRY 🧚      ║${RESET}"
echo -e "${PINK}╚═══════════════════════════╝${RESET}"
echo

step_done() {
	echo -e "${GREEN}Done${RESET}"
}

step_fail() {
	echo -e "${RED}Failed${RESET}"
	if [[ -n "${1:-}" ]]; then
		echo -e "${RED}$1${RESET}"
	fi
	exit 1
}

step_info() {
	echo -e "$1"
}

echo -e -n "📝 Checking norm..."
NORM_OUTPUT=$(find . -type d -name "$(basename "$TESTER_DIR")" -prune -o \
    \( -name "*.c" -o -name "*.h" \) -type f -print | xargs -r norminette 2>&1)

if echo "$NORM_OUTPUT" | grep -q "Error"; then
    NORM_TEST_RES=1
    echo -e "\t  ${RED}Failed${RESET}"
    echo ""
    echo "$NORM_OUTPUT" | grep "Error"
    echo ""
else
    NORM_TEST_RES=0
    echo -e "\t    Done"
fi

echo -e -n "🔖 Checking version..."
if grep -qi "bonus:" Makefile; then
	BONUS_VERSION=1
	echo -e "\t   Bonus"
else
	BONUS_VERSION=0
	echo -e " ${YELLOW}Mandatory${RESET}"
fi

echo -e -n "📋 Checking prototype..."
regex="int\t+ft_printf\(const char \*format, \.\.\.\)"
expected="int\tft_printf(const char *format, ...)"
if ! grep -Pq "$regex" ft_printf.h; then
    echo -e "  ${RED}Failed${RESET}"
    echo ""
    echo -e "Missing or malformed prototype(s), expected:\n$expected"
    echo ""
else
    echo -e "    Done"
fi

echo -e -n "⚙️  Checking Makefile..."
MAKE_ERRORS=0
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
BUILD_ERRORS=0
BUILD_ISSUES=""
MAKE_OUTPUT=$(make re 2>&1)
if [ $? -ne 0 ]; then
    ((BUILD_ERRORS++))
    BUILD_ISSUES+="Command 'make re' failed:\n$MAKE_OUTPUT\n"
fi
if [ ! -f "libftprintf.a" ]; then
    ((BUILD_ERRORS++))
    BUILD_ISSUES+="File 'libftprintf.a' was not produced.\n"
fi
if [ $BUILD_ERRORS -eq 0 ]; then
    echo -e "    Done"
else
    echo -e "  ${RED}Failed${RESET}"
    echo ""
    echo -e "$BUILD_ISSUES"
    echo ""
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
for obj in "$PROJECT_DIR"/*.o; do
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
TESTER_NAME=$(basename "$TESTER_DIR")
CFLAGS="-Wall -Wextra -Wno-format -I."
LDFLAGS="-L. -lftprintf"
WRAP_FLAGS="-Wl,--wrap=malloc"
BUILD_ERRORS=0
BUILD_ISSUES=""
COMPILE_OUT=$(cc $CFLAGS -DVERBOSE=$VERBOSE "$TESTER_NAME/basic_tests.c" "$TESTER_NAME/utils.c" -o "$TESTER_NAME/basic_test" $WRAP_FLAGS $LDFLAGS 2>&1)
if [ $? -ne 0 ]; then
    ((BUILD_ERRORS++))
    BUILD_ISSUES+="basic_tests compilation failed:\n$COMPILE_OUT\n"
fi
COMPILE_OUT=$(cc $CFLAGS "$TESTER_NAME/leak_tests.c" "$TESTER_NAME/utils.c" -o "$TESTER_NAME/leak_test" $WRAP_FLAGS $LDFLAGS 2>&1)
if [ $? -ne 0 ]; then
    ((BUILD_ERRORS++))
    BUILD_ISSUES+="leak_tests compilation failed:\n$COMPILE_OUT\n"
fi
if [ $BUILD_ERRORS -eq 0 ]; then
    echo -e "\t    Done"
else
    echo -e "\t  ${RED}Failed${RESET}"
    echo ""
    echo -e "$BUILD_ISSUES"
    echo ""
    exit 1
fi

echo -e -n "🧪 Running tests..."
basic_log="$TESTER_NAME/.basic.log"
leak_log="$TESTER_NAME/.leak.log"
"$TESTER_NAME/basic_test" > "$basic_log" 2>&1
BASIC_RC=$?
valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all \
    --error-exitcode=42 --track-origins=yes \
    "$TESTER_NAME/leak_test" >/dev/null 2>"$leak_log"
LEAK_RC=$?
if [[ $BASIC_RC -eq 0 && $LEAK_RC -eq 0 ]]; then
    echo -e "\t    Done"
else
    echo -e "\t  ${RED}Failed${RESET}"
fi
if [[ $BASIC_RC -ne 0 || $VERBOSE -eq 1 ]]; then
    echo ""
    cat "$basic_log"
fi
if [[ $LEAK_RC -ne 0 || $VERBOSE -eq 1 ]]; then
    echo ""
    echo "════════════════════════════════════════"
    echo "VALGRIND OUTPUT (mandatory)"
    echo "════════════════════════════════════════"
    cat "$leak_log"
    echo ""
fi


BONUS_BASIC_RC=0
BONUS_LEAK_RC=0
if [[ $BONUS_VERSION -eq 1 ]]; then
    echo -e -n "📦 Building bonus..."
    BUILD_ERRORS=0
    BUILD_ISSUES=""
    MAKE_OUTPUT=$(make re bonus 2>&1)
    if [ $? -ne 0 ]; then
        ((BUILD_ERRORS++))
        BUILD_ISSUES+="Command 'make re bonus' failed:\n$MAKE_OUTPUT\n"
    fi
    if [ $BUILD_ERRORS -eq 0 ]; then
        echo -e "\t    Done"
    else
        echo -e "\t  ${RED}Failed${RESET}"
        echo ""
        echo -e "$BUILD_ISSUES"
        echo ""
    fi

    echo -e -n "🔨 Building bonus tests..."
    BUILD_ERRORS=0
    BUILD_ISSUES=""
    COMPILE_OUT=$(cc $CFLAGS -DVERBOSE=$VERBOSE "$TESTER_NAME/basic_tests_bonus.c" "$TESTER_NAME/utils.c" -o "$TESTER_NAME/basic_test_bonus" $WRAP_FLAGS $LDFLAGS 2>&1)
    if [ $? -ne 0 ]; then
        ((BUILD_ERRORS++))
        BUILD_ISSUES+="basic_tests_bonus compilation failed:\n$COMPILE_OUT\n"
    fi
    COMPILE_OUT=$(cc $CFLAGS "$TESTER_NAME/leak_tests_bonus.c" "$TESTER_NAME/utils.c" -o "$TESTER_NAME/leak_test_bonus" $WRAP_FLAGS $LDFLAGS 2>&1)
    if [ $? -ne 0 ]; then
        ((BUILD_ERRORS++))
        BUILD_ISSUES+="leak_tests_bonus compilation failed:\n$COMPILE_OUT\n"
    fi
    if [ $BUILD_ERRORS -eq 0 ]; then
        echo -e "  Done"
    else
        echo -e "${RED}Failed${RESET}"
        echo ""
        echo -e "$BUILD_ISSUES"
        echo ""
        exit 1
    fi

    echo -e -n "🧪 Running bonus tests..."
    bonus_basic_log="$TESTER_NAME/.basic_bonus.log"
    bonus_leak_log="$TESTER_NAME/.leak_bonus.log"
    "$TESTER_NAME/basic_test_bonus" > "$bonus_basic_log" 2>&1
    BONUS_BASIC_RC=$?
    valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all \
        --error-exitcode=42 --track-origins=yes \
        "$TESTER_NAME/leak_test_bonus" >/dev/null 2>"$bonus_leak_log"
    BONUS_LEAK_RC=$?
    if [[ $BONUS_BASIC_RC -eq 0 && $BONUS_LEAK_RC -eq 0 ]]; then
        echo -e "   Done"
    else
        echo -e " ${RED}Failed${RESET}"
    fi
    if [[ $BONUS_BASIC_RC -ne 0 || $VERBOSE -eq 1 ]]; then
        echo ""
        cat "$bonus_basic_log"
    fi
    if [[ $BONUS_LEAK_RC -ne 0 || $VERBOSE -eq 1 ]]; then
        echo ""
        echo "════════════════════════════════════════"
        echo "VALGRIND OUTPUT (bonus)"
        echo "════════════════════════════════════════"
        cat "$bonus_leak_log"
        echo ""
    fi
fi

echo ""
if [[ $BASIC_RC -eq 0 && $LEAK_RC -eq 0 && $BONUS_BASIC_RC -eq 0 && $BONUS_LEAK_RC -eq 0 && $NORM_TEST_RES -eq 0 && $MAKE_ERRORS -eq 0 ]]; then
    echo -e "${GREEN}╔══════════════════════════════╗${RESET}"
    echo -e "${GREEN}║      OH MY, YOU PASSED!      ║${RESET}"
    echo -e "${GREEN}╚══════════════════════════════╝${RESET}"
    exit 0
else
    echo -e "${RED}╔══════════════════════════════╗${RESET}"
    echo -e "${RED}║     OH NO... YOU FAILED!     ║${RESET}"
    echo -e "${RED}╚══════════════════════════════╝${RESET}"
    exit 1
fi
