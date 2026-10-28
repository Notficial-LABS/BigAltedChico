#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
set -euo pipefail

project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
action="${1:-build}"
build_dir="$project_dir/Working/build"
cd "$project_dir"
case "$action" in build|check|preview|install) ;; *) echo 'Usage: ./build.sh [build|check|preview|install]' >&2; exit 2 ;; esac

# Optional convenience for the original Mac development setup. No user-specific path.
juce_dir="${JUCE_DIR:-}"
if [[ -z "$juce_dir" && -f "$HOME/Applications/JUCE-9.0.4/CMakeLists.txt" ]]; then
    juce_dir="$HOME/Applications/JUCE-9.0.4"
fi
args=(-S "$project_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release)
if [[ -n "$juce_dir" ]]; then args+=("-DJUCE_DIR=$juce_dir"); fi
if [[ "$(uname -s)" == Darwin ]]; then args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=12.0); fi
if [[ "$action" == check ]]; then args+=(-DBIGCHICO_BUILD_CHECKS=ON); fi
cmake "${args[@]}"

case "$action" in
    check)
        mkdir -p Working/Previews Working/Tests
        cmake --build "$build_dir" --config Release --target BigAltedChicoChecks BigAltedChicoHostChecks BigAltedChico_VST3 --parallel "${BUILD_JOBS:-6}"
        "$build_dir/BigAltedChicoChecks_artefacts/Release/BigAltedChicoChecks" "$project_dir/Working/Previews/preview.png"
        "$build_dir/BigAltedChicoHostChecks_artefacts/Release/BigAltedChicoHostChecks" \
            "$build_dir/BigAltedChico_artefacts/Release/VST3/BigAltedChico.vst3"
        ;;
    preview)
        if [[ "$(uname -s)" != Darwin ]]; then
            echo 'Live preview helper is macOS-only; use CMake and launch the standalone directly.' >&2
            exit 2
        fi
        cmake --build "$build_dir" --config Release --target BigAltedChico_Standalone --parallel "${BUILD_JOBS:-6}"
        osascript -e 'if application "BigAltedChico" is running then tell application "BigAltedChico" to quit' >/dev/null 2>&1 || true
        open -n "$build_dir/BigAltedChico_artefacts/Release/Standalone/BigAltedChico.app"
        ;;
    build|install)
        targets=(BigAltedChico_Standalone BigAltedChico_VST3)
        if [[ "$(uname -s)" == Darwin ]]; then targets+=(BigAltedChico_AU); fi
        cmake --build "$build_dir" --config Release --target "${targets[@]}" --parallel "${BUILD_JOBS:-6}"
        if [[ "$action" == install ]]; then
            if [[ "$(uname -s)" != Darwin ]]; then echo 'The local install helper is macOS-only.' >&2; exit 2; fi
            install_dir="$HOME/Library/Audio/Plug-Ins/VST3"
            mkdir -p "$install_dir"
            if [[ -d "$install_dir/BigAltedChico.vst3" ]]; then
                backup_dir="$project_dir/Working/Backups/installed-$(date +%Y%m%d-%H%M%S)"
                mkdir -p "$backup_dir"
                mv "$install_dir/BigAltedChico.vst3" "$backup_dir/BigAltedChico.vst3"
            fi
            ditto "$build_dir/BigAltedChico_artefacts/Release/VST3/BigAltedChico.vst3" "$install_dir/BigAltedChico.vst3"
            codesign --verify --deep --strict "$install_dir/BigAltedChico.vst3"
            echo "Installed VST3: $install_dir/BigAltedChico.vst3"
        fi
        ;;
esac
