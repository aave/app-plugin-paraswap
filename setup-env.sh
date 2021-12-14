# shellcheck disable=SC1091,SC2155

# SOURCE THIS FILE
# . prepare-devenv blue|s|x
#This script is used to setup the toolchain and also the target PSD

    echo "**************************************************************************"
    echo "           P R E P A R I N G    N A N O     E N V I R O N M E N T"
    echo "**************************************************************************"
    echo " "
if [ $# -ne 1 ]; then
    echo "Possible options: blue, s or x"
    return
elif [[ $1 == "-h" ]]; then
    echo "Possible options: blue, s or x"
    return
elif [[ $1 != "blue" ]] && [[ $1 != "s" ]] && [[ $1 != "x" ]]; then
    echo "Possible options: blue, s or x"
    return
fi

#Checking if the appropriate version of python3-venv is installed and installing it if it is not the case
if [[ $(dpkg-query -s python3-venv 2>&1) == *'is not installed'* ]]; then
    echo "**************************************************************************"
    echo "                      Installing Python"
    echo "**************************************************************************"
    echo " "
    sudo apt install python3-venv

fi

#Checking if udev rules are added and adding them if it is not the case
#if [[ $(cat /etc/udev/rules.d/20-hw1.rules) == *'ATTRS{idVendor}=="2c97", ATTRS{idProduct}=="0004"'* ]]; then
    echo "**************************************************************************"
    echo "                      Installing Udev rules"
    echo "**************************************************************************"
    echo " "
    wget -q -O - https://raw.githubusercontent.com/LedgerHQ/udev-rules/master/add_udev_rules.sh | sudo bash

#fi

if [ ! -d dev-env ]; then
    echo "**************************************************************************"
    echo "                      Creating components"
    echo "**************************************************************************"
    echo " "
    echo "                      Folder Structure ..."
    mkdir dev-env
    mkdir dev-env/SDK
    mkdir dev-env/CC
    mkdir dev-env/CC/others
    mkdir dev-env/CC/nanox
    
    sudo apt install make

    echo "                      installing libncurses5 ..."
    
    #sudo apt install libncurses5
    echo "                      installing gcc-multilib, g++-multilib ..."
    
    sudo apt install gcc-multilib g++-multilib

    echo "                      installing gcc-arm-none-eabi ..."
    

    wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2
    mv gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2 dev-env/CC/others/
    cd dev-env/CC/others/
    tar xf gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2
    rm -Rf gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2
    cp -r gcc-arm-none-eabi-10-2020-q4-major ../nanox/gcc-arm-none-eabi-10-2020-q4-major
    cd -

    echo "                      Downloading clang"
    
    wget http://releases.llvm.org/7.0.0/clang+llvm-7.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz -O clang+llvm.tar.xz
    tar xf clang+llvm.tar.xz
    rm clang+llvm.tar.xz
    cp -r clang+llvm* dev-env/CC/others/clang-arm-fropi
    mv clang+llvm* dev-env/CC/nanox/clang-arm-fropi

    #This is not required as the ledger blue is no longer supported
    #wget https://github.com/LedgerHQ/blue-secure-sdk/archive/blue-r21.1.tar.gz -O blue-secure-sdk.tar.gz
    #tar xf blue-secure-sdk.tar.gz
    #rm blue-secure-sdk.tar.gz
    #mv blue-secure-sdk* dev-env/SDK/blue-secure-sdk

#Cloning and placing the nano S SDK
    echo "                      cloning nanos-secure-sdk"
    git clone https://github.com/LedgerHQ/nanos-secure-sdk.git
    mv nanos-secure-sdk* dev-env/SDK/nanos-secure-sdk

    python3 -m venv dev-env/ledger_py3
    source dev-env/ledger_py3/bin/activate
    echo "                      installing wheel"
    pip install wheel
    echo "                      installing ledgerblue"
    pip install ledgerblue
fi

echo "**************************************************************************"
echo "                      Setting environment variables"
echo "**************************************************************************"
echo " "

source dev-env/ledger_py3/bin/activate

if [[ $1 == "blue" ]]; then
    export BOLOS_SDK=$(pwd)/dev-env/SDK/blue-secure-sdk
    export BOLOS_ENV=$(pwd)/dev-env/CC/others
elif [[ $1 == "s" ]]; then
    export BOLOS_SDK=$(pwd)/dev-env/SDK/nanos-secure-sdk
    export BOLOS_ENV=$(pwd)/dev-env/CC/others
elif [[ $1 == "x" ]]; then
    export BOLOS_SDK=$(pwd)/dev-env/SDK/nanox-secure-sdk
    export BOLOS_ENV=$(pwd)/dev-env/CC/nanox
fi

    echo "                      S U C C E S S"
    echo "**************************************************************************"


