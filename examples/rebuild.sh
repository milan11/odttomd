#!/bin/sh

set -u
set -e

ODTTOMD_PATH="../../odttomd-build/debug/odttomd"

for ODT_FILE in *.odt
do
	echo ">> ${ODT_FILE}"
	FILE_NAME=$(basename "${ODT_FILE}" ".odt")
	"${ODTTOMD_PATH}" "${ODT_FILE}" > "${FILE_NAME}.md"
done

