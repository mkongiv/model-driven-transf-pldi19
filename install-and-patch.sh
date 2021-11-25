#!/bin/bash

if [ $# -ne 1 ]; then
  echo "Usage: ./install-and-patch.sh <install-folder>"
  exit 1
fi

tgt=$1

if [ -d $tgt ]; then
  echo "Target folder exists. Removing..."
  rm -rf $tgt
fi

mkdir $tgt

poccball=pocc-1.5.0-beta-selfcontained.tar.gz
cp $poccball $tgt/

cp patches/ponos-0.3.0.tar.gz $tgt/
cp patches/punroller-0.3.1.tar.gz $tgt/

cd $tgt
tar xvzf $poccball

poccdir="pocc-1.5.0-beta"
cd $poccdir

cp ../*.tar.gz archives/

if [ $? -ne 0 ]; then
  echo "Error copying tarball patches. Aborting..."
  exit 1
fi

tpath=driver/src/
cp ../../patches/$tpath/*.[ch] $tpath/

if [ $? -ne 0 ]; then
  echo "Error copying [ch]-files to $tpath/. Aborting..."
  exit 1
fi

tpath=driver/pocc/
cp ../../patches/$tpath/*.c $tpath

if [ $? -ne 0 ]; then
  echo "Error copying c-sources to $tpath/. Aborting..."
  exit 1
fi

tpath=driver/pocc/include/pocc
cp ../../patches/$tpath/*.h $tpath/

if [ $? -ne 0 ]; then
  echo "Error copying headers to $tpath. Aborting..."
  exit 1
fi

./install.sh

tpath=optimizers/ponos/ponos/
cp ../../patches/iutil.c $tpath/
bin/pocc-util make all

make install

cp ../../run-mdt.sh .
cp ../../gemm.c .

./run-mdt.sh gemm 7 KNL
