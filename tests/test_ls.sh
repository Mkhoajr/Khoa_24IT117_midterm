#!/bin/sh

# NetBSD ls(1) Midterm - Stage 6 Test Script

set -u

cd "$(dirname "$0")/.." || exit 1

if [ ! -x ./myls ]; then
    echo "ERROR: myls not found. Run make first."
    exit 1
fi

TEST_DIR=$(mktemp -d /tmp/myls-test.XXXXXX) || exit 1

trap 'rm -rf "$TEST_DIR"' 0
trap 'exit 1' 1 2 3 15

mkdir -p "$TEST_DIR/sample/sub/inner" || exit 1

printf 'hello\n' > "$TEST_DIR/sample/a.txt"
printf 'world\n' > "$TEST_DIR/sample/sub/b.txt"
printf 'deep\n' > "$TEST_DIR/sample/sub/inner/c.txt"

touch "$TEST_DIR/sample/.hidden"
chmod +x "$TEST_DIR/sample/a.txt"

ln -s a.txt "$TEST_DIR/sample/link.txt" || exit 1

PASS=0
FAIL=0

check_run()
{
    LABEL=$1
    shift

    if ./myls "$@" > /dev/null 2>&1; then
        echo "PASS: $LABEL"
        PASS=$((PASS + 1))
    else
        echo "FAIL: $LABEL"
        FAIL=$((FAIL + 1))
    fi
}

check_match()
{
    LABEL=$1
    shift

    ./myls "$@" > "$TEST_DIR/actual" 2> "$TEST_DIR/actual.err"
    MY_STATUS=$?

    ls -1 "$@" > "$TEST_DIR/expected" 2> "$TEST_DIR/expected.err"
    LS_STATUS=$?

    if [ "$MY_STATUS" -eq 0 ] &&
       [ "$LS_STATUS" -eq 0 ] &&
       cmp -s "$TEST_DIR/expected" "$TEST_DIR/actual"
    then
        echo "PASS: $LABEL"
        PASS=$((PASS + 1))
    else
        echo "FAIL: $LABEL"
        FAIL=$((FAIL + 1))

        diff -u "$TEST_DIR/expected" "$TEST_DIR/actual" || :
    fi
}

echo "======================================"
echo "NetBSD ls(1) - Stage 6 Tests"
echo "======================================"

echo ""
echo "[1] Basic functionality"

check_run "Basic listing" "$TEST_DIR/sample"
check_run "Hidden entries (-a)" -a "$TEST_DIR/sample"
check_run "Almost all (-A)" -A "$TEST_DIR/sample"
check_run "Long format (-l)" -l "$TEST_DIR/sample"
check_run "Numeric IDs (-n)" -n "$TEST_DIR/sample"
check_run "Classification (-F)" -F "$TEST_DIR/sample"
check_run "Inode numbers (-i)" -i "$TEST_DIR/sample"
check_run "Size sorting (-S)" -S "$TEST_DIR/sample"
check_run "Time sorting (-t)" -t "$TEST_DIR/sample"
check_run "Reverse sorting (-r)" -r "$TEST_DIR/sample"
check_run "Human sizes (-lh)" -lh "$TEST_DIR/sample"
check_run "Block sizes (-sk)" -sk "$TEST_DIR/sample"

echo ""
echo "[2] Recursive listing"

check_run "Recursive (-R)" -R "$TEST_DIR/sample"
check_run "Long recursive (-lR)" -lR "$TEST_DIR/sample"
check_run "Hidden recursive (-aR)" -aR "$TEST_DIR/sample"

echo ""
echo "[3] Filename handling"

check_run "Question mode (-q)" -q "$TEST_DIR/sample"
check_run "Raw mode (-w)" -w "$TEST_DIR/sample"
check_run "Option order (-wq)" -wq "$TEST_DIR/sample"
check_run "Option order (-qw)" -qw "$TEST_DIR/sample"

echo ""
echo "[4] Multiple operands"

check_run "Multiple paths" \
    "$TEST_DIR/sample/a.txt" \
    "$TEST_DIR/sample/sub" \
    "$TEST_DIR/sample"

echo ""
echo "[5] Compare with NetBSD ls"

check_match "Default listing" "$TEST_DIR/sample"
check_match "Reverse order" -r "$TEST_DIR/sample"
check_match "Sort by size" -S "$TEST_DIR/sample"
check_match "Classification" -F "$TEST_DIR/sample"

echo ""
echo "[6] Error handling"

if ./myls "$TEST_DIR/does-not-exist" > /dev/null 2>&1; then
    echo "FAIL: Invalid path"
    FAIL=$((FAIL + 1))
else
    echo "PASS: Invalid path"
    PASS=$((PASS + 1))
fi

echo ""
echo "======================================"
echo "PASS: $PASS"
echo "FAIL: $FAIL"
echo "======================================"

if [ "$FAIL" -eq 0 ]; then
    echo "PASS: basic smoke tests"
    exit 0
fi

echo "FAIL: some tests failed"
exit 1
