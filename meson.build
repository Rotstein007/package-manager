project('package-manager', ['cpp', 'c'],
        version: '0.1.0',
        meson_version: '>= 1.0.0',
        default_options: ['warning_level=2', 'werror=false', 'cpp_std=gnu++2a'],
)

# Definiere die Abhängigkeit für gtkmm-4.0
gtkmm_dep = dependency('gtkmm-4.0', required: true)

# Die Quellen des Projekts
package_manager_sources = [
    'src/main.cpp',
]

# Erstelle die ausführbare Datei mit den Abhängigkeiten
executable('package-manager', package_manager_sources,
           dependencies: gtkmm_dep,
           install: true,
)

# Update the URL to the new repository
git_clone_url = 'https://github.com/sass/sassc.git'
git_branch = 'master'  # Use the correct branch name

# Clone the repository
git = find_program('git', required: true)
run_command(git, 'clone', '--depth', '1', '--branch', git_branch, git_clone_url, 'sassc')