%define ver 0.4.4

Name: nailer
Summary: Thumbnail Generator that uses mplayer
Version: %{ver}
Release: 1.f%{fedora}
License: GPL
Group: Multimedia
Packager: Kevin DeKorte <kdekorte@gmail.com>
Source0: http://dekorte.homeip.net/download/nailer/nailer-%{ver}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
Requires: mplayer
BuildRequires: glib2-devel, pkgconfig, gtk2-devel, GConf2

%description
Thumbnail Generator that uses mplayer

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
rm -rf %buildroot
make install DESTDIR=%buildroot

%clean
rm -rf $buildroot

%post
/sbin/ldconfig
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
gconftool-2 --makefile-install-rule %{_sysconfdir}/gconf/schemas/nailer.schemas >/dev/null || :

%files
%defattr(-,root,root,-)
%{_docdir}/%{name}-%{version}
%{_sysconfdir}/gconf/schemas/nailer.schemas
%{_prefix}/bin/nailer
%{_prefix}/share/thumbnailers/video-thumbnailer.desktop

