# 3D Arcade — PS2 homebrew source

Four fixed-view, 3D arcade-style games in one PlayStation 2 homebrew ELF. This is original, simplified game code inspired by Pac-Man, Tetris, Space Invaders and Pong, not ports of their commercial releases.

## Important: current status

**Experimental and not yet confirmed working.** A previous ELF displayed only a blue screen in PCSX2. A diagnostic confirmed that the screen and controller worked but the test triangles did not show. This revision turns off GS AA1, which is a possible cause of invisible triangles; the revised build still needs to be tested in PCSX2 and on real hardware. A successful GitHub Actions build only means it compiled; it does not prove it runs or plays correctly. No BIOS or other console firmware is included.

## Build automatically on GitHub

1. Create a new empty GitHub repository.
2. Upload **all extracted contents** of this folder to the repository root. Include the hidden `.github/workflows/build.yml` file: without it, the automatic build will not appear. Do not upload just the ZIP.
3. After the upload is committed, open the repository's **Actions** tab. If requested, enable Actions. Select **Build PS2 ELF** and open the latest run. You can also start one with **Run workflow**.
4. When the run has a green check mark, scroll to **Artifacts**, download **3D-Arcade-PS2-ELF**, and extract `ARCADE3D.ELF` from it.
5. Open the ELF in PCSX2, or use a homebrew launcher on a real PS2.

GitHub Actions needs internet access to download the official prebuilt ps2dev toolchain. The workflow uses the toolchain's `latest` release, so future toolchain changes may affect the build.

If using Git on your computer instead of uploading files in the browser, run these commands inside the extracted project folder, replacing the URL with your actual repository URL:

```sh
git init
git add .
git commit -m "Add PS2 arcade source"
git branch -M main
git remote add origin YOUR_REPOSITORY_URL
git push -u origin main
```

## Build locally

Install [ps2dev](https://github.com/ps2dev/ps2dev), including PS2SDK and gsKit. Then:

```sh
export PS2DEV=/path/to/ps2dev
make PS2DEV="$PS2DEV"
```

This produces `ARCADE3D.ELF`. The PS2 compiler and toolchain are not included in the repository.

## Controls

- Menu: D-pad selects, X starts.
- All games: Start pauses; Up/Down and X select Resume or Game Select. Start also resumes.
- Pac-Man: D-pad moves; larger pellets briefly enable ghost-eating.
- Tetris: Left/Right move, Up rotates, Down soft-drops, X hard-drops, and Circle holds or swaps the current piece (once per piece).
- Space Invaders: Left/Right move; hold Up to fire.
- Pong: Up/Down move the left paddle.

There is no audio or save support. Each run restarts when chosen from the menu. The games have no designed final stage or winning score; counters remain finite.

## Running the ELF

PCSX2 requires a PS2 BIOS legally dumped from your own console. Configure the controller in port 1 and use the emulator's Start File / Run ELF command.

On a PS2, copy the ELF to a FAT32 USB drive, then launch it from `mass:` using an already-installed homebrew launcher such as wLaunchELF. An unmodified console cannot normally launch an ELF directly. Physical-console compatibility has not been verified.

## Host tests and diagnostics

`tests/test_logic.c` checks menu selection, pause handling, scoring and several progression/reset paths on a desktop. It does not emulate PS2 graphics. The GitHub workflow runs it before compiling. The `diagnostic/` folder contains the separate controller/graphics test program; it is not the game.
