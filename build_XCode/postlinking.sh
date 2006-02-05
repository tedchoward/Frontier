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

# Strip trademark character to prevent problems wuth shell quoting
#product_str=`echo ${PRODUCT_NAME} | awk -F '\342' '{print $1};'`

product_str=${PRODUCT_NAME}

# Parse the "versions.h" file for useful variables

version_str=`grep "frontier_version_string" "${SRCROOT}/../Common/headers/versions.h" | awk -F \" '{print $2};'`
copyright_year_str=`grep "#define[[:space:]]*copyright_year_string" "${SRCROOT}/../Common/headers/versions.h" | awk -F \" '{print $2};'`

# Update the Info.plist
# Could append "(dbg)" to product name for Development build style

#defaults write "${BUILT_PRODUCTS_DIR}/${CONTENTS_FOLDER_PATH}/Info" CFBundleVersion ${version_str}

# or use sed

sed -e "s/\${PRODUCT_NAME_STR}/${product_str}/g" -e "s/\${PRODUCT_VERSION_STR}/${version_str}/g" -e "s/\${COPYRIGHT_YEAR_STR}/${copyright_year_str}/g" "${INFOPLIST_FILE}" > "${BUILT_PRODUCTS_DIR}/${CONTENTS_FOLDER_PATH}/Info.plist"
