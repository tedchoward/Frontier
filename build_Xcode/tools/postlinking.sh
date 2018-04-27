# debug
#set -x
#printenv

# Build Frontier.icns file
# There seems to be some bugs here, so just use
# pre-created Frontier.icns file.
# Also add "icns.r" path to Input files and
# "Frontier.icns" path to Output files below.
#
#/Developer/Tools/Rez "${SRCROOT}/../resources/Frontier/icns.r" -d TARGET_API_MAC_CARBON=1 -d FRONTIER_FRAMEWORK_INCLUDES=1 -i "${SRCROOT}/../Common/headers" -useDF -o "${BUILT_PRODUCTS_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/Frontier.icns"

# Strip trademark character to prevent problems with shell quoting
#product_str=`echo ${PRODUCT_NAME} | awk -F '\342' '{print $1};'`

product_str=${PRODUCT_NAME}


##################################################
#
# Parse the "versions.h" file for useful variables
#

echo "Extracting info from Common/headers/versions.h for Info.plist file"

# go to tools directory
currdir=`pwd`
cd "${SRCROOT}/tools/"

# pick up preprocessor definitions from target settings, e.g. "PIKE=1"
preprocessor_defs=`echo ${GCC_PREPROCESSOR_DEFINITIONS} | sed -e "s/[A-Z0-9_][A-Z0-9_=]*/-D&/g"`

# set environment variable to stop cc from complaining
export MACOSX_DEPLOYMENT_TARGET="10.6"

# extract the APP_VERSION_STRING
echo "cc ${preprocessor_defs} -isysroot ${SDKROOT} -I \"${SRCROOT}/../Common/headers\" -o appversion appversion.c"
cc ${preprocessor_defs} -isysroot ${SDKROOT} -I"${SRCROOT}/../Common/headers" -o appversion appversion.c
version_str=`./appversion`
echo "Using version string: ${version_str}"

# extract the copyright_year_string
echo "cc ${preprocessor_defs} -isysroot ${SDKROOT} -I \"${SRCROOT}/../Common/headers\" -o appcopyright appcopyright.c"
cc ${preprocessor_defs} -isysroot ${SDKROOT} -I"${SRCROOT}/../Common/headers" -o appcopyright appcopyright.c
copyright_str=`./appcopyright`
echo "Using copyright string: ${copyright_str}"

# clean up
rm -f appversion appcopyright

# revert to previous working directory
cd "${currdir}"

##################################################


# build info.plist file using extracted information

sed -e "s/\${PRODUCT_NAME_STR}/${product_str}/g" \
	-e "s/\${PRODUCT_VERSION_STR}/${version_str}/g" \
	-e "s/\${PRODUCT_COPYRIGHT_STR}/${copyright_str}/g" \
	"${INFOPLIST_FILE}" > "${BUILT_PRODUCTS_DIR}/${CONTENTS_FOLDER_PATH}/Info.plist"
