#!/bin/sh

set -u
set -e

ODTTOMD_PATH="../../odttomd-build/debug/odttomd"

for PROFILE in standard github plain_text readable parseable
do
	echo "---"
	echo "PROFILE: ${PROFILE}"
	mkdir -p "${PROFILE}"
	for ODT_FILE in *.odt
	do
		echo ">> ${ODT_FILE}"
		FILE_NAME=$(basename "${ODT_FILE}" ".odt")
		"${ODTTOMD_PATH}" --profile "${PROFILE}" "${ODT_FILE}" > "${PROFILE}/${FILE_NAME}.md"
	done
done

