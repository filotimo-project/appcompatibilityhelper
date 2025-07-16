Name:          filotimo-appcompatibilityhelper
Version:       0.11
Release:       1%{?dist}
License:       GPL-2.0-or-later
Summary:       Provides support for running or finding alternatives to certain package types on ublue-based distributions.
URL:           https://github.com/filotimo-project/appcompatibilityhelper
Source:        %{name}-%{version}.tar.gz

BuildRequires: extra-cmake-modules
BuildRequires: gcc-c++
BuildRequires: kf6-rpm-macros
BuildRequires: desktop-file-utils
BuildRequires: gettext

BuildRequires: cmake(Qt6Core)
BuildRequires: cmake(Qt6Gui)
BuildRequires: cmake(Qt6Qml)
BuildRequires: cmake(Qt6QuickControls2)
BuildRequires: cmake(Qt6Svg)
BuildRequires: cmake(Qt6Xml)
BuildRequires: cmake(Qt6Widgets)

BuildRequires: cmake(KF6Kirigami)
BuildRequires: cmake(KF6CoreAddons)
BuildRequires: cmake(KF6I18n)
BuildRequires: cmake(KF6KIO)

Requires: qt6qml(org.kde.coreaddons)
Requires: qt6qml(org.kde.kirigami)
Requires: qt6qml(org.kde.kirigamiaddons.formcard)
Requires: rpm
Requires: cpio
Requires: binutils
Requires: tar


%description
Provides support for running or finding alternatives to certain package types on ublue-based distributions.

%prep
%setup -q

%build
%cmake_kf6
%cmake_build

%install
%cmake_install
# no translations yet
#%find_lang %{name} --with-man --with-qt --all-name

%check
desktop-file-validate %{buildroot}%{_kf6_datadir}/applications/org.filotimoproject.appcompatibilityhelper.desktop

# -f %{name}.lang - no translations yet
%files
%license LICENSES/*
%{_kf6_bindir}/appcompatibilityhelper
%{_kf6_datadir}/applications/org.filotimoproject.appcompatibilityhelper.desktop
%{_kf6_datadir}/appcompatibilityhelper/app_db.json

%changelog
* Wed Jul 16 2025 Thomas Duckworth <tduck@filotimoproject.org> 0.11-1
- chore: update windows strings (tduck@filotimoproject.org)
- Update README.MD (tduck@filotimoproject.org)
- chore: update potfile (tduck@filotimoproject.org)

* Wed Jul 16 2025 Thomas Duckworth <tduck@filotimoproject.org> 0.10-2
- fix: pull proper dependencies (tduck@filotimoproject.org)

* Wed Jul 16 2025 Thomas Duckworth <tduck@filotimoproject.org> 0.10-1
- new package built with tito

