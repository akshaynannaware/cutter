#!/bin/sh

script_base_dir=`dirname $0`

if [ $# != 5 ]; then
    echo "Usage: $0 PACKAGE PACKAGE_TITLE BASE_URL_PREFIX DISTRIBUTIONS HAVE_DEVELOPMENT_BRANCH"
    echo " e.g.: $0 milter-manager 'milter manager' http://downloads.sourceforge.net/milter-manager' 'fedora centos' yes"
    exit 1
fi

PACKAGE=$1
PACKAGE_TITLE=$2
BASE_URL_PREFIX=$3
DISTRIBUTIONS=$4
HAVE_DEVELOPMENT_BRANCH=$5

run()
{
    "$@"
    if test $? -ne 0; then
	echo "Failed $@"
	exit 1
    fi
}

rpm_base_dir=$HOME/rpm

if [ ! -f ~/.rpmmacros ]; then
    run cat <<EOM > ~/.rpmmacros
%_topdir $rpm_base_dir
EOM
fi

run mkdir -p $rpm_base_dir/SOURCES
run mkdir -p $rpm_base_dir/SPECS
run mkdir -p $rpm_base_dir/BUILD
run mkdir -p $rpm_base_dir/RPMS
run mkdir -p $rpm_base_dir/SRPMS

for distribution in ${DISTRIBUTIONS}; do
    case $distribution in
	fedora)
	    distribution_label=Fedora
	    ;;
	centos)
	    distribution_label=CentOS
	    ;;
    esac
    repo=${PACKAGE}.repo
    if test "$HAVE_DEVELOPMENT_BRANCH" = "yes"; then
	run cat <<EOR > $repo
[$PACKAGE]
name=$PACKAGE_TITLE for $distribution_label \$releasever - \$basearch
baseurl=$BASE_URL_PREFIX/$distribution/\$releasever/stable/\$basearch/
gpgcheck=1
enabled=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-$PACKAGE

[$PACKAGE-development]
name=$PACKAGE_TITLE for $distribution_label \$releasever - development - \$basearch
baseurl=$BASE_URL_PREFIX/$distribution/\$releasever/development/\$basearch/
gpgcheck=1
enabled=0
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-$PACKAGE
EOR
    else
	run cat <<EOR > $repo
[$PACKAGE]
name=$PACKAGE_TITLE for $distribution_label \$releasever - \$basearch
baseurl=$BASE_URL_PREFIX/$distribution/\$releasever/\$basearch/
gpgcheck=1
enabled=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-$PACKAGE
EOR
    fi
    run tar cfz $rpm_base_dir/SOURCES/${PACKAGE}-repository.tar.gz \
	-C ${script_base_dir} ${repo} RPM-GPG-KEY-${PACKAGE}
    run cp ${script_base_dir}/${PACKAGE}-repository.spec $rpm_base_dir/SPECS/

    run rpmbuild -ba $rpm_base_dir/SPECS/${PACKAGE}-repository.spec

    top_dir=$script_base_dir/$distribution

    run mkdir -p $top_dir
    run cp -p $rpm_base_dir/RPMS/noarch/${PACKAGE}-repository-* $top_dir
    run cp -p $rpm_base_dir/SRPMS/${PACKAGE}-repository-* $top_dir

    run cp -p ${script_base_dir}/RPM-GPG-KEY-${PACKAGE} $top_dir
done
