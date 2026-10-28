# Licensing

## Original project material: MIT

Copyright (c) 2026 Leonardo Marcelo / Notficial LABS.

The original BigAltedChico source, tools, documentation and original project artwork are under the MIT license in [LICENSE](../LICENSE). Modification, redistribution and commercial use are permitted under its terms, including closed-source reuse of that original code. Keep the copyright and permission notice. Contributors submit their original contributions under the same terms unless explicitly agreed otherwise.

The MIT notice does not relicense third-party software, fonts, or other dependencies. Brand artwork identifies Notficial LABS; distributing a modified build does not make it an official Notficial LABS release.

## Dependencies and combined binaries

| Material | License | Notice |
| --- | --- | --- |
| Signalsmith Stretch | MIT | `ThirdParty/SignalsmithStretch-LICENSE.txt` |
| Signalsmith Linear | MIT | `ThirdParty/signalsmith-linear/LICENSE.txt` |
| Roboto | SIL OFL 1.1 | `Assets/Roboto-OFL.txt` |
| JUCE 9 | AGPLv3 or commercial JUCE license | `Licenses/JUCE-LICENSE.md` |

JUCE is external to this source repository. The intended official open-source binary distribution uses JUCE's AGPLv3 option. Combining our MIT-licensed code with JUCE does **not** make the complete application MIT-only: the combined binary distribution must comply with the applicable AGPL conditions, including providing corresponding source and notices. Its original MIT notices remain intact.

A downstream author wanting to distribute a closed-source JUCE-based binary needs a suitable commercial JUCE license; our MIT license cannot grant rights to JUCE. Private modification does not by itself create a requirement to publish a release.

Each open-source binary release should provide the matching source, build scripts, dependency sources/versions and license texts required to rebuild it. Use `Licenses/AGPL-3.0.txt` for the AGPL text; its upstream text was obtained from the SPDX license list. Do not rely solely on an automatic source archive that omits the external JUCE source.

Official references:

- JUCE license: https://github.com/juce-framework/JUCE/blob/master/LICENSE.md
- GNU AGPLv3: https://www.gnu.org/licenses/agpl-3.0.html
- GNU license guidance: https://www.gnu.org/licenses/gpl-howto.html
- SPDX license text: https://github.com/spdx/license-list-data/blob/main/text/AGPL-3.0-only.txt

VST2 and AAX are excluded from this project and its release plan. The ordinary VST3 dependencies supplied by JUCE retain their upstream license notices.
