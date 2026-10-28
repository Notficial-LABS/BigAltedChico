# Contributing to BigAltedChico

Bug reports, code improvements, tests, documentation, translations and custom skins are welcome. You do not need a compiler to improve documentation or create a JSON/SVG skin.

## Fork and clone

1. Open [Notficial-LABS/BigAltedChico](https://github.com/Notficial-LABS/BigAltedChico) on GitHub and select **Fork**.
2. Clone your fork. Replace `YOUR-USERNAME` with your GitHub username:

   ```sh
   git clone https://github.com/YOUR-USERNAME/BigAltedChico.git
   cd BigAltedChico
   git remote add upstream https://github.com/Notficial-LABS/BigAltedChico.git
   ```

3. Create a focused branch. For example, for a documentation improvement:

   ```sh
   git switch -c docs/improve-readme
   ```

`origin` is your fork, where you push your work. `upstream` is the official project. Cloning the official repository is enough to explore/build it; a fork gives you a place to publish a proposed change.

## Make and check a change

For C++ changes, follow [BUILDING](docs/BUILDING.md). Keep generated files in ignored `Working/`. New skin designs start with the [Skin Quickstart](SkinTemplate/QUICKSTART.md); include editable artwork, a unique skin ID, installation instructions and credits.

Use validation appropriate to the change:

| Change | Expected checks |
| --- | --- |
| Documentation | Preview the Markdown, check links and examples. |
| Custom skin | Import/reload it; check all three UI sizes, control states, settings and session restore. |
| Plugin UI | Check Juguete, Ondas and Salem at 75%, 100%, 125%, including numeric editing, toggles and modal dismissal. |
| DSP | Compare Quality/Live with low/high notes, fast note changes and transients; verify dry/wet/bypass alignment and actual/reported host latency. |
| State or presets | Check save/load, DAW session reopening and compatibility with existing state. |
| Installer | Check only supported formats, installation scope, selected components, upgrade and uninstall on the actual target OS. |

Describe the platforms you actually tested. Keep known limitations explicit. Preserve plugin IDs, parameter IDs and license notices. A state-format change needs a compatibility or migration plan.

## Commit, push and open a pull request

Review the changed files:

```sh
git status
git diff
```

For the README example, stage and commit the edited file, then push the branch:

```sh
git add README.md
git commit -m "docs: improve setup and download instructions"
git push -u origin docs/improve-readme
```

For another change, stage the relevant source files and use your own branch name and commit message. Check the staged diff with `git diff --cached` before committing.

On GitHub, open your fork and choose **Compare & pull request**, or use the official project's **Pull requests → New pull request → compare across forks**. Set the base repository to `Notficial-LABS/BigAltedChico`, base branch `main`, and the head to your fork/branch.

Explain the problem, the resulting behaviour, how to reproduce/check it, and what you tested. Include screenshots for visual changes when useful. Small, focused pull requests are easier to review.

## Update your fork

With a clean working tree, bring your local `main` up to date before starting another branch:

```sh
git fetch upstream
git switch main
git merge --ff-only upstream/main
git push origin main
```

If `--ff-only` reports a divergence, inspect your local commits before merging/rebasing; do not discard your work. Create your next topic branch from the updated `main`.

## Report a bug

Open an [Issue](https://github.com/Notficial-LABS/BigAltedChico/issues). Include:

- OS version and architecture.
- Plugin version, format and DAW/version, or Standalone audio device.
- Sample rate, buffer size, tuning mode and Live/Quality setting where relevant.
- Steps to reproduce, expected result and actual result.
- A screenshot or concise log excerpt when helpful.

## Files and licensing

Do not commit builds, installers, signing materials, personal presets, DAW sessions, audio renders or machine-specific paths. `Working/` is ignored. Review `git status` and the staged diff before pushing.

Original contributions are MIT-licensed under [LICENSE](LICENSE). Dependencies keep their respective licenses; see [LICENSING](docs/LICENSING.md). Include sources and applicable notices for third-party artwork used in skins.

GitHub's [fork guide](https://docs.github.com/en/pull-requests/how-tos/work-with-forks/fork-a-repo) and [pull-request guide](https://docs.github.com/en/pull-requests/how-tos/create-pull-requests/creating-a-pull-request-from-a-fork) explain the platform workflow in more detail.
