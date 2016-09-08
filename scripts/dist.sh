#!/bin/sh
echo "Hengband dist script for OS X"
echo "usage: dist.sh [revision]"

# configure
local_repo_dir=~/Projects
revision=${1:-master}
work_dir=./work-$revision
remote_repo_hengband=git://git.osdn.jp/gitroot/hengband/hengband.git
repo_hengband=$local_repo_dir/hengband.git
remote_repo_hengband_cocoa=https://github.com/shimitei/hengband-cocoa.git
repo_hengband_cocoa=$local_repo_dir/hengband-cocoa.git

# required
echo "required command checking..."
required () {
    if type $1 > /dev/null 2>&1; then
        echo "\t$1: exists"
    else
        echo "\t$1: not exists"
        exit 1
    fi
}
echo "required: Git client"
required git
echo "required: Xcode & Command Line Tools"
required clang
required ld
echo "required: nkf"
required nkf

if [ -e $work_dir ]; then
  echo "ERROR: already exist working directory!"
  echo "Please remove directory: rm -rf $work_dir"
  exit 1
fi

# git fetch
mkdir -p $local_repo_dir 2>/dev/null
fetch_git () {(
    if [ ! -e $2 ]; then
        cd $local_repo_dir
        git clone --bare $1
    else
        cd $2
        git fetch origin 'refs/heads/*:refs/heads/*'
    fi
)}
fetch_git $remote_repo_hengband $repo_hengband
fetch_git $remote_repo_hengband_cocoa $repo_hengband_cocoa

# git checkout
mkdir -p $work_dir
checkout_git () {(
    mkdir -p $3
    cd $3
    dest_dir=`pwd`
    cd $1
    git cat-file -t $2 || return 1
    git archive --format=tar $2 | tar -C $dest_dir -xf -
)}
checkout_hengband () {(
    cd $work_dir
    checkout_git $repo_hengband $revision hengband || return 1
    checkout_git $repo_hengband_cocoa master hengband-cocoa
)}
checkout_hengband || exit 1

# patch
(
    cd $work_dir
    \cp -fr hengband-cocoa/src/cocoa hengband/src
    \cp -f hengband-cocoa/src/main-cocoa.m hengband/src
    \cp -f hengband-cocoa/src/makefile.osx hengband/src
    \cp -f hengband-cocoa/lib/pref/pref-mac.prf hengband/lib/pref
    \cp -fr hengband-cocoa/scripts hengband
    \cp -f hengband-cocoa/readme_mac_jp.txt hengband
    time_now=`date "+%Y%m%d%H%M"`
    echo "$time_now-$revision" > hengband/version
    cd hengband/src
    nkf -e --overwrite *.c
    nkf -e --overwrite *.h
)

# dist
(
    cd $work_dir
    cd hengband/src
    make -f makefile.osx dist
)
