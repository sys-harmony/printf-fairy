#!/bin/bash
# printf-fairy: a tester for 42's ft_printf (subject v12.1).
# Clone it inside your ft_printf repository, then: ./run.sh [-v|--verbose]

# ═════════════════════════════════ fairy core ═════════════════════════════════
# Identical in libft-fairy, printf-fairy and gnl-fairy: display, steps, the
# checks the three subjects share, and the parallel test runner.

set -u

RED=$'\033[31m' GREEN=$'\033[32m' YELLOW=$'\033[33m' PINK=$'\033[95m'
BOLD=$'\033[1m' DIM=$'\033[2m' RESET=$'\033[0m'
WIDTH=46                        # banner width; every status word ends there
JOBS=$(nproc 2> /dev/null || echo 4)
VERBOSE=0

for arg in "$@"; do
    case $arg in
        -v|--verbose) VERBOSE=1 ;;
        -h|--help)    echo "usage: $0 [-v|--verbose]"; exit 0 ;;
        *)            echo "usage: $0 [-v|--verbose]" >&2; exit 2 ;;
    esac
done

TESTER_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
TESTER=$(basename "$TESTER_DIR")
cd "$TESTER_DIR/.." || exit 1
TMP=$(mktemp -d "/tmp/$TESTER.XXXXXX") || exit 1
export FAIRY_STATS=$TMP/stats   # each test binary appends "passed failed warned"
FAILED=0 WARNED=0 FAILED_STEPS="" START=$(date +%s%N)
declare -A PIDS

# ---- display -----------------------------------------------------------------
# box <color> <text> [emoji]: a WIDTH-wide double-line box, text centered
box() {
    local text=$2 w=${#2} bar l
    [[ -n ${3:-} ]] && text+=" $3" w=$((w + 3))
    printf -v bar '%*s' $((WIDTH - 2)) ''
    l=$(( (WIDTH - 2 - w) / 2 ))
    printf '%s╔%s╗\n║%*s%s%*s║\n╚%s╝%s\n' "$1" "${bar// /═}" $l '' "$text" \
        $((WIDTH - 2 - w - l)) '' "${bar// /═}" "$RESET"
}

# step <emoji> <label> opens a step line; ok, fail, warn or note closes it with
# a dot leader and a status word ending in column WIDTH. Details go below.
step() { STEP=$2 STEP_COL=$((${#2} + 4)); printf '%s %s ' "$1" "$2"; }
status() {
    local dots
    printf -v dots '%*s' $((WIDTH - STEP_COL - ${#2} - 1)) ''
    printf '%s%s%s %s%s%s\n' "$DIM" "${dots// /.}" "$RESET" "$1" "$2" "$RESET"
}
ok()   { status "$GREEN" Done; }
fail() { FAILED=1 FAILED_STEPS+="${FAILED_STEPS:+, }$STEP"; status "$RED" Failed; }
warn() { WARNED=1; status "$YELLOW" Warning; }
note() { status "$BOLD" "$1"; }
# verdict <issues...>: fails with one line per issue, or succeeds
verdict() { if [[ $# -eq 0 ]]; then ok; else fail; for i in "$@"; do say "$i"; done; fi; }

say()   { printf '   ↳ %s\n' "$*"; }          # one detail line under a step
quote() { sed 's/^/     /'; }                  # indents what it reads

# section <title>: the header of a block of results, as test binaries print it
section() {
    local line n=$((WIDTH - ${#1} - 4))
    printf -v line '%*s' $((n > 3 ? n : 3)) ''
    printf '\n%s──%s %s%s%s %s%s%s\n' "$DIM" "$RESET" "$BOLD" "$1" "$RESET" "$DIM" "${line// /─}" "$RESET"
}

# ---- end of run: cleanup and verdict -----------------------------------------
clean_project() { :; }
kill_tree() { local c; for c in $(pgrep -P "$1" 2> /dev/null); do kill_tree "$c"; done; kill "$1" 2> /dev/null; }

finish() {
    local rc=$? pid total=0 ko=0 secs
    for pid in $(pgrep -P $$ 2> /dev/null); do kill_tree "$pid"; done
    if [[ $rc -eq 130 ]]; then
        clean_project; rm -rf "$TMP"
        printf '\n%sInterrupted: no verdict.%s\n\n' "$YELLOW" "$RESET"
        return
    fi
    step 🧹 Cleanup; clean_project; ok
    [[ -f $FAIRY_STATS ]] && read -r total ko < <(awk '{p += $1; f += $2} END {print p + f, f + 0}' "$FAIRY_STATS")
    rm -rf "$TMP"
    secs=$(( ($(date +%s%N) - START) / 10000000 ))
    printf -v secs '%d.%02ds' $((secs / 100)) $((secs % 100))
    echo
    if [[ $FAILED -eq 0 ]]; then
        box "$GREEN" "OH MY, YOU PASSED!"
        printf '   %s✔ %d tests passed in %s%s' "$GREEN" "$total" "$secs" "$RESET"
        [[ $WARNED -eq 1 ]] && printf '  %s⚠ warnings above: not failures, worth a look%s' "$YELLOW" "$RESET"
    else
        box "$RED" "OH NO... YOU FAILED!"
        printf '   %s✘ %s' "$RED" "$FAILED_STEPS"
        [[ $ko -gt 0 ]] && printf ' · %d/%d tests failed' "$ko" "$total"
        printf ' · %s%s' "$secs" "$RESET"
    fi
    printf '\n\n'
}
trap finish EXIT
trap 'exit 130' INT TERM

# ---- background jobs ---------------------------------------------------------
# job <name> <cmd...>: runs cmd in the background, at most JOBS at once. Its
# output lands in $TMP/<name>.log, its exit status in $TMP/<name>.rc.
job() {
    local name=$1
    shift
    while [[ $(jobs -rp | wc -l) -ge $JOBS ]]; do wait -n; done
    { "$@" > "$TMP/$name.log" 2>&1; echo $? > "$TMP/$name.rc"; } &
    PIDS[$name]=$!
}
await() { local n; for n in "$@"; do wait "${PIDS[$n]}" 2> /dev/null; done; }
rc() { cat "$TMP/$1.rc" 2> /dev/null || echo 1; }

# ---- checks shared by the three subjects -------------------------------------
in_git() { git rev-parse --is-inside-work-tree > /dev/null 2>&1; }

# sources: the .c and .h files a grader gets. In a git repository that already
# tracks some: the tracked ones (a local main.c is fine). Otherwise all.
sources() {
    local files=""
    in_git && files=$(git ls-files -- '*.c' '*.h' | grep -v "^$TESTER/")
    [[ -z $files ]] && files=$(find . -path "./$TESTER" -prune -o -type f \( -name '*.c' -o -name '*.h' \) -print | sed 's|^\./||' | sort)
    [[ -n $files ]] && printf '%s\n' "$files"
}

# report_untracked <pathspecs...>: warns about the project files git would not
# hand to the grader. Returns 1 when there is none.
report_untracked() {
    local files
    files=$(in_git && git ls-files -o --exclude-standard -- "$@" | grep -v "^$TESTER/" | xargs)
    [[ -z $files ]] && return 1
    warn; say "not committed, hence not graded: $files"
}

# Norminette starts in the background at once: it is the slowest check.
norm_start() { [[ -n $(sources) ]] && job norm norminette $(sources); }
check_norm() {
    step 📝 Norm
    await norm
    if ! command -v norminette > /dev/null; then
        warn; say "norminette not found: the norm was NOT checked"
    elif [[ -z $(sources) ]]; then
        fail; say "no .c or .h file to check"
    elif [[ $(rc norm) -eq 0 ]]; then
        ok
    else
        fail
        grep -Ev 'OK!$|^Setting locale' "$TMP/norm.log" \
            | awk 'NR <= 20; END {if (NR > 20) print "… " NR - 20 " more lines"}' | quote
    fi
}

# check_readme [topic]: README.md as the subjects describe it. A missing one
# fails; its shape only warns, as a pattern cannot read prose.
check_readme() {
    local issues=() section
    step 📖 README
    if [[ ! -f README.md ]]; then
        fail; say "no README.md at the root of the repository"; return
    fi
    head -n 1 README.md | grep -qE '^\s*(\*|_|<i>|<em>)This project has been created as part of the 42 curriculum by ' \
        || issues+=("first line, in italics: *This project has been created as part of the 42 curriculum by <login>*")
    for section in Description Instructions Res+ources; do
        grep -qiE "^\s*(#+|[*_]{1,2}).*$section" README.md || issues+=("no \"${section/s+/s}\" section")
    done
    grep -qiE '\bAI\b|artificial intelligence|chatgpt|claude|copilot|gemini|LLM' README.md \
        || issues+=("Resources: say how AI was used, for which tasks and parts")
    [[ -n ${1:-} ]] && ! grep -qi "$1" README.md && issues+=("no explanation of the $1")
    if [[ ${#issues[@]} -eq 0 ]]; then
        ok
    else
        warn; for section in "${issues[@]}"; do say "$section"; done
    fi
}

# emitted <symbol>: a call the compiler emits on its own (array initializer,
# struct copy, stack protector), never written in the sources (comments aside)
emitted() {
    [[ -f $TMP/code ]] || sources | grep '\.c$' | xargs -r cat \
        | { perl -0777 -pe 's{/\*.*?\*/}{ }gs; s{//[^\n]*}{}g' 2> /dev/null || cat; } > "$TMP/code"
    [[ $1 =~ ^(memset|memcpy|memmove|__stack_chk_fail|_GLOBAL_OFFSET_TABLE_)$ ]] && ! grep -qw -- "$1" "$TMP/code"
}

# ---- test runs ---------------------------------------------------------------
# tests <title> <binary>: a test binary prints the sections that did not pass
# and exits 0, 1 (failure) or 2 (warnings). Anything else is reported here.
tests() {
    local rc
    (timeout --foreground 600 "$2" 2>&1) 2> /dev/null     # no "Segmentation fault" from bash
    rc=$?
    [[ $rc -le 2 ]] && return $rc
    section "$1"
    if [[ $rc -eq 124 ]]; then say "timed out"; else say "crashed: SIG$(kill -l $((rc - 128)) 2> /dev/null)"; fi
    if [[ -n ${LENIENT:-} ]]; then echo "1 0 1" >> "$FAIRY_STATS"; return 2; fi
    echo "0 1 0" >> "$FAIRY_STATS"
    return 1
}

# memcheck <title> <binary> [args...]: runs it under Valgrind, where VG_LEAKS
# are the leak kinds that fail. Prints the report if it found something.
memcheck() {
    local log=$TMP/vg.$BASHPID rc note=""
    if ! command -v valgrind > /dev/null; then
        echo "1 0 1" >> "$FAIRY_STATS"
        section "valgrind · $1"; say "valgrind not found: leaks NOT checked"
        return 2
    fi
    (timeout --foreground 30 valgrind --leak-check=full --show-leak-kinds=all \
        --errors-for-leak-kinds="$VG_LEAKS" --error-exitcode=42 --track-origins=yes \
        --log-file="$log" "${@:2}" > /dev/null 2>&1) 2> /dev/null
    rc=$?
    if [[ $rc -ne 0 ]] && grep -q "very large text, data or bss" "$log"; then
        rc=2 note="Valgrind cannot load it (huge static data): leaks NOT checked"
    elif [[ $rc -eq 0 ]] && grep -qE "(still reachable|possibly lost): [1-9]" "$log"; then
        rc=2 note="memory still held at exit: not a leak for Valgrind, worth freeing"
    elif [[ $rc -eq 124 ]]; then
        rc=1 note="timed out"
    elif [[ $rc -ne 0 && $rc -ne 42 ]]; then
        note="crashed: SIG$(kill -l $((rc - 128)) 2> /dev/null)" rc=1
    elif [[ $rc -eq 42 ]]; then
        rc=1
    fi
    [[ $rc -eq 1 && -n ${LENIENT:-} ]] && rc=2
    echo "$((rc != 1)) $((rc == 1)) $((rc == 2))" >> "$FAIRY_STATS"
    [[ $rc -eq 0 && $VERBOSE -eq 0 ]] && return 0
    section "valgrind · $1"
    [[ -n $note ]] && say "$note"
    # The report without its banner, pid prefixes and hints
    sed -E 's/^==[0-9]+== ?//' "$log" | awk '
        /^If you believe this happened/ {skip = 1} skip {skip = !/stack size used/; next}
        /^(Memcheck,|Copyright|Using Valgrind|Command:|Parent PID:)/ || /rerun with/ {next}
        /^$/ {if (!blank++ && shown) print; next} {blank = 0; shown = 1; print}' | quote
    return $rc
}

# lenient <cmd...>: runs tests or memcheck where a failure only warns
# (outside the subject, or debatable)
lenient() { LENIENT=1 "$@"; }

# queue <list> <name> <cmd...>: starts a run in the background, in a named
# list. run_queue <list> waits for the list, closes the step with the worst
# status and prints the reports in order.
queue() { local list=$1; shift; eval "Q_$list+=(\"\$1\")"; job "$@"; }
run_queue() {
    local -n runs=Q_$1
    local name worst=0 code
    await "${runs[@]}"
    for name in "${runs[@]}"; do
        code=$(rc "$name")
        if [[ $code -eq 2 ]]; then [[ $worst -eq 0 ]] && worst=2; elif [[ $code -ne 0 ]]; then worst=1; fi
    done
    case $worst in 0) ok ;; 2) warn ;; *) fail ;; esac
    for name in "${runs[@]}"; do cat "$TMP/$name.log"; done
}

# ═══════════════════════════════ end of core ══════════════════════════════════

NAME=libftprintf.a
VG_LEAKS=all            # no static state in ft_printf: any memory left is a leak
clean_project() { make fclean > /dev/null 2>&1; }

echo; box "$PINK" "PRINTF-FAIRY" 🧚; echo
norm_start

step 🔖 Version
BONUS=0
grep -Eqs '^bonus[[:space:]]*:' Makefile && BONUS=1
if [[ $BONUS -eq 1 ]]; then note Bonus; else note Mandatory; fi

# Project files: the Makefile, the README, every header and every source the
# Makefile compiles (a local main.c is not one)
step 📂 Files
HEADER=$(sources | grep -E '(^|/)ft_printf\.h$' | head -n 1)
CMDS=$( { make -nB; [[ $BONUS -eq 1 ]] && make -nB bonus; } 2> /dev/null)
COMPILED=$(grep -oE '[^[:space:]/]+\.c\b' <<< "$CMDS" | sort -u | sed 's|.*|&\n*/&|')
issues=()
[[ -f Makefile ]] || issues+=("no Makefile at the root of the repository")
[[ -n $HEADER ]] || issues+=("no ft_printf.h, the header the subject asks for")
if [[ ${#issues[@]} -gt 0 ]]; then verdict "${issues[@]}"
elif ! report_untracked Makefile README.md '*.h' $COMPILED; then ok; fi

check_readme "algorithm"
check_norm

# Compiled against the subject's prototype: a missing declaration is an
# undeclared identifier, a different one a conflicting type.
step 📋 Prototype
INCLUDES=$(sources | grep '\.h$' | xargs -rn1 dirname | sort -u | sed 's/^/-I/')
if [[ -z $HEADER ]]; then
    fail; say "no ft_printf.h"
elif printf '#include "%s"\nint (*fairy)(const char *, ...) = &ft_printf;\nint ft_printf(const char *, ...);\n' \
        "$(basename "$HEADER")" | cc -fsyntax-only $INCLUDES -x c - 2> "$TMP/proto.err"; then
    ok
else
    fail; say "$HEADER must declare: int ft_printf(const char *, ...);"
    grep -m 3 'error' "$TMP/proto.err" | quote
fi

# Rules and flags as make really runs them (make -n), variables expanded
step 🔧 Makefile
[[ -f Makefile ]] || { fail; say "no Makefile"; exit 1; }
rule() { grep -qE "^([^#:=]+[[:space:]])?($1)([[:space:]][^#:=]*)?[[:space:]]*:([^=]|$)" Makefile; }
issues=()
rule '\$[({]NAME[)}]|libftprintf\.a' || issues+=("no \$(NAME) rule")
for r in all clean fclean re; do rule "$r" || issues+=("no '$r' rule"); done
grep -vE '^\s*#' Makefile | grep -qE '\$\((wildcard|shell)|^[^#[:space:]][^#]*=[^#]*\*\.c' \
    && issues+=("sources listed with a wildcard: name each file")
COMPILES=$(grep -E '(^|\s)-c(\s|$)' <<< "$CMDS")
if [[ -z $COMPILES ]]; then
    issues+=("make -n shows no compilation")
else
    grep -vqE '(^|[;&|/[:space:]])cc\s' <<< "$COMPILES" && issues+=("sources not compiled with cc")
    for flag in -Wall -Wextra -Werror; do
        grep -vq -- "$flag" <<< "$COMPILES" && issues+=("sources compiled without $flag")
    done
fi
grep -qE '(^|[;&|/[:space:]])libtool\s' <<< "$CMDS" && issues+=("libtool is forbidden")
grep -qE '(^|[;&|/[:space:]])ar\s' <<< "$CMDS" || issues+=("$NAME not created with ar")
verdict "${issues[@]}"

# build <label> [goal]: a build from scratch (in parallel; sequential if that
# fails, as some valid Makefiles do not support -j), then the same goal
# again, which must not relink
build() {
    local stamp
    step 📦 "$1"
    make fclean > /dev/null 2>&1
    if ! { make -j"$JOBS" ${2:-} > /dev/null 2>&1 && [[ -f $NAME ]]; } \
        && ! { make fclean > /dev/null 2>&1; make ${2:-} > "$TMP/make.log" 2>&1 && [[ -f $NAME ]]; }; then
        fail; tail -n 15 "$TMP/make.log" | quote
        [[ -f $NAME ]] || say "'make${2:+ $2}' did not create $NAME at the root"
        exit 1
    fi
    stamp=$(stat -c %y "$NAME")
    make ${2:-} > /dev/null 2>&1
    if [[ $(stat -c %y "$NAME") == "$stamp" ]]; then ok; else fail; say "relinks: a second 'make${2:+ $2}' rebuilds $NAME"; fi
}

# Reads the archive itself, so objects built in a subfolder (libft) count too.
# Allowed: malloc, free, write; va_* are builtins.
externals() {
    local sym found=()
    step 🔍 "$1"
    for sym in $(comm -23 <(nm -u "$NAME" 2> /dev/null | awk '$1 == "U" {print $2}' | sort -u) \
            <(nm --defined-only "$NAME" 2> /dev/null | awk 'NF == 3 {print $3}' | sort -u)); do
        [[ $sym =~ ^(malloc|free|write)$ ]] || emitted "$sym" || found+=("$sym, called in $(nm -A -u "$NAME" \
            | awk -v s="$sym" '$NF == s {split($1, p, ":"); print p[2]}' | sort -u | xargs)")
    done
    [[ ${#found[@]} -gt 0 ]] && found=("${found[@]/#/forbidden: }")
    verdict "${found[@]}"
}

# build_tests <label> <tag> <tests...>: links each test file with the library
build_tests() {
    local label=$1 tag=$2 t errors=""
    shift 2
    step 🔨 "$label"
    for t in "$@"; do
        job "cc_$tag$t" cc -g -Wall -Wextra -Wno-format -DVERBOSE=$VERBOSE "$TESTER/$t.c" "$TESTER/utils.c" \
            -o "$TMP/$tag$t" -no-pie -L. -lftprintf -Wl,--wrap=malloc
    done
    for t in "$@"; do
        await "cc_$tag$t"
        [[ $(rc "cc_$tag$t") -eq 0 ]] || errors+="$t.c:"$'\n'"$(head -n 12 "$TMP/cc_$tag$t.log")"$'\n'
    done
    if [[ -z $errors ]]; then ok; else fail; printf '%s' "$errors" | quote; exit 1; fi
}

# The runs of a part start as soon as it is built, in the background: the
# mandatory tests run while the bonus builds.
build "Build"
externals "External functions"
build_tests "Test build" m_ basic_tests leak_tests
queue main m_basic tests "basic_tests" "$TMP/m_basic_tests"
queue main m_leak memcheck "leak_tests" "$TMP/m_leak_tests"

if [[ $BONUS -eq 1 ]]; then
    # The mandatory tests run again: a bonus often rewrites every conversion
    build "Bonus build" bonus
    externals "Bonus external functions"
    build_tests "Bonus test build" b_ basic_tests basic_tests_bonus leak_tests leak_tests_bonus
    queue bonus b_basic tests "basic_tests (bonus build)" "$TMP/b_basic_tests"
    queue bonus b_basic_bonus tests "basic_tests_bonus" "$TMP/b_basic_tests_bonus"
    queue bonus b_leak memcheck "leak_tests (bonus build)" "$TMP/b_leak_tests"
    queue bonus b_leak_bonus memcheck "leak_tests_bonus" "$TMP/b_leak_tests_bonus"
fi

step 🧪 Tests
run_queue main
if [[ $BONUS -eq 1 ]]; then
    step 🧪 "Bonus tests"
    run_queue bonus
fi
exit $FAILED
