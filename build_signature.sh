set -e
test -f $HOME/.gitconfig || echo ".gitconfig not found in current home: $HOME"

BUILD_INFO_OUT="$1"
BUILD_TAG="$2"
echo "generating build signature"

GIT_ORIGIN_URL="$(git config --get remote.origin.url)"

GIT_DESCRIBE="$(git describe --long)"
if [ -z "$GIT_DESCRIBE" ] ; then GIT_DESCRIBE="0.0.0-0-g0000000" ; fi

GIT_TAG="$(echo "$GIT_DESCRIBE" | awk -F"-" '{print $1;}')"
if [ -z "$GIT_TAG" ] ; then GIT_TAG="none" ; fi

GIT_VERSION="$(echo "$GIT_DESCRIBE" | awk -F"-" '{print $1;}')"
if [ -z "$GIT_VERSION" ] ; then GIT_VERSION="0.0.0" ; fi
if ! echo "$GIT_VERSION" | grep -Eq '^[0-9]+\.[0-9]+\.[0-9]+$' ; then GIT_VERSION="0.0.0" ; fi

GIT_VERSION_MAJOR="$(echo "$GIT_VERSION" | awk -F"." '{print $1;}')"
GIT_VERSION_MINOR="$(echo "$GIT_VERSION" | awk -F"." '{print $2;}')"
GIT_VERSION_PATCH="$(echo "$GIT_VERSION" | awk -F"." '{print $3;}')"
if [ -z "$GIT_VERSION_MAJOR" ] ; then GIT_VERSION_MAJOR="0" ; fi
if [ -z "$GIT_VERSION_MINOR" ] ; then GIT_VERSION_MINOR="0" ; fi
if [ -z "$GIT_VERSION_PATCH" ] ; then GIT_VERSION_PATCH="0" ; fi

GIT_COMMITS_NUMBER="$(echo "$GIT_DESCRIBE" | awk -F"-" '{print $2;}')"
if [ -z "$GIT_COMMITS_NUMBER" ] ; then GIT_COMMITS_NUMBER="0" ; fi

GIT_HASH="$(git rev-parse HEAD)"
if [ -z "$GIT_HASH" ] ; then GIT_HASH="0000000000000000000000000000000000000000" ; fi

GIT_HASH_SHORT="$(echo "$GIT_HASH" | head -c 8)"
if [ -z "$GIT_HASH_SHORT" ] ; then GIT_HASH_SHORT="00000000" ; fi

GIT_DIFF="$(git diff --shortstat)"
if [ -z "$GIT_DIFF" ] ; then 
    GIT_DIFF_LINES=0
else
    GIT_DIFF_FILES="$(echo "$GIT_DIFF" | cut -d"," -f1)"
    GIT_DIFF_1="$(echo "$GIT_DIFF" | cut -d"," -f2)"
    GIT_DIFF_2="$(echo "$GIT_DIFF" | cut -d"," -f3)"
    GIT_DIFF_FILES="$(echo "$GIT_DIFF_FILES" | cut -d" " -f2)"
    GIT_DIFF_1="$(echo "$GIT_DIFF_1" | cut -d" " -f2)"
    GIT_DIFF_2="$(echo "$GIT_DIFF_2" | cut -d" " -f2)"
    if [ -z "$GIT_DIFF_2" ] ; then GIT_DIFF_2=0 ; fi
    let "GIT_DIFF_LINES = $GIT_DIFF_1 + $GIT_DIFF_2"
fi

GIT_BRANCH="$(git rev-parse --abbrev-ref HEAD)"
if [ -z "$GIT_BRANCH" ] ; then GIT_BRANCH="0" ; fi
GIT_BRANCH_CHAR="$(echo "$GIT_BRANCH" | head -c 1)"

GIT_USERNAME="$(git config user.name)"
GIT_USEREMAIL="$(git config user.email)"

ENV_UNIX_TIME_UTC="$(date "+%s")"
if [ -z "$ENV_UNIX_TIME_UTC" ] ; then ENV_UNIX_TIME_UTC=0 ; fi
ENV_DATE_LOCAL="$(date "+%Y-%m-%d" -d @"$ENV_UNIX_TIME_UTC")"
ENV_TIME_LOCAL="$(date "+%H:%M:%S" -d @"$ENV_UNIX_TIME_UTC")"

cat > "$BUILD_INFO_OUT" <<DELIM
// This file is auto-generated, triggered by project build,
// do not change it manually !
#ifndef _BUILD_SIGNATURE_H_
#define _BUILD_SIGNATURE_H_

#define GIT_ORIGIN_URL       "${GIT_ORIGIN_URL}"
#define GIT_DESCRIBE         "${GIT_DESCRIBE}"
#define GIT_TAG              "${GIT_TAG}"
#define GIT_VERSION          "${GIT_VERSION}"
#define GIT_VERSION_FULL     "${GIT_VERSION}-${GIT_COMMITS_NUMBER}-${GIT_DIFF_LINES}-${GIT_BRANCH_CHAR}-${GIT_HASH_SHORT}-${ENV_UNIX_TIME_UTC}"
#define GIT_HASH             "${GIT_HASH}"
#define GIT_HASH_SHORT_STR   "${GIT_HASH_SHORT}"
#define GIT_BRANCH           "${GIT_BRANCH}"
#define GIT_BRANCH_CHAR_STR  "${GIT_BRANCH_CHAR}"
#define GIT_BRANCH_CHAR_NUM  '${GIT_BRANCH_CHAR}'

#define GIT_VERSION_MAJOR    (${GIT_VERSION_MAJOR})
#define GIT_VERSION_MINOR    (${GIT_VERSION_MINOR})
#define GIT_VERSION_PATCH    (${GIT_VERSION_PATCH})
#define GIT_COMMITS_NUMBER   (${GIT_COMMITS_NUMBER})
#define GIT_DIFF_NUMBER      (${GIT_DIFF_LINES})

#define GIT_USERNAME         "${GIT_USERNAME}"
#define GIT_USEREMAIL        "${GIT_USEREMAIL}"
#define ENV_UNIX_TIME_UTC    (${ENV_UNIX_TIME_UTC})
#define ENV_DATE_LOCAL       "${ENV_DATE_LOCAL}"
#define ENV_TIME_LOCAL       "${ENV_TIME_LOCAL}"
#define ENV_USERNAME         "${USER}"
#define ENV_HOSTNAME         "${HOSTNAME}"

#define BUILD_TAG            "${BUILD_TAG}"

#endif  // _BUILD_SIGNATURE_H_

DELIM

echo "$BUILD_INFO_OUT is generated"
