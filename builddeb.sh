#!/bin/sh
debuild -b
fakeroot debian/rules clean
rm debian/changelog
rm debian
