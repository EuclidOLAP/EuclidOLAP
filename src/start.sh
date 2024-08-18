#!/bin/bash

###############################################################################
#  Sets the environment variable for the absolute path to the bin directory,  #
#  which will be used by the EuclidOLAP service process.                      #
###############################################################################
bin_dir=$(
cd $(dirname "${BASH_SOURCE[0]}")
pwd
)

# echo $bin_dir

export EUCLIDOLAP_BIN_DIR=$bin_dir

###############################################################################
##  Determine the current operating system.                                   #
###############################################################################
current_os=""

if [[ "$OSTYPE" == "linux-gnu" ]]; then
    if [ -f /etc/redhat-release ]; then
        current_os="redhat"
        # echo "Redhat Linux detected."
    elif [ -f /etc/SuSE-release ]; then
        current_os="suse"
        # echo "Suse Linux detected."
    elif [ -f /etc/arch-release ]; then
        current_os="arch"
        # echo "Arch Linux detected."
    elif [ -f /etc/mandrake-release ]; then
        current_os="mandrake"
        # echo "Mandrake Linux detected."
    elif [ -f /etc/debian_version ]; then
        current_os="debian"
        echo "Ubuntu/Debian Linux detected."
    else
        current_os="unknown linux distribution"
        # echo "Unknown Linux distribution."
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    current_os="mac/darwin"
    # echo "Mac OS (Darwin) detected."
elif [[ "$OSTYPE" == "freebsd"* ]]; then
    current_os="freebsd"
    # echo "FreeBSD detected."
else
    current_os="unknown operating system"
    # echo "Unknown operating system."
fi

###############################################################################
#  Execute specific instructions depending on the operating system type.      #
###############################################################################
if [[ "$current_os" == "redhat" ]]; then
    ${EUCLIDOLAP_BIN_DIR}/euclid-svr-redhat
elif [[ "$current_os" == "debian" ]]; then
    ${EUCLIDOLAP_BIN_DIR}/euclid-svr-debian
else
    echo "Currently, EuclidOLAP supports running on Redhat/CentOS and Debian/Ubuntu operating systems."
    echo "Your OS:" $current_os
    echo "The program has exited."
    exit 1
fi