

from pathlib import Path
import platform
import shutil

from invoke.tasks import task


ROOT = Path(__file__).parent.resolve()
CONFIG_DIR = ROOT / "config"
BUILD_DIR = ROOT / "build"


def _default_profile() -> Path:
    system = platform.system().lower()
    if system == "windows":
        return CONFIG_DIR / "windows.profile"
    if system == "linux":
        return CONFIG_DIR / "linux.profile"
    raise RuntimeError(f"Unsupported system: {system}")


def _conan_install(
    ctx,
    build_type: str,
    profile: Path,
    conan_conf: list[str] | None,
) -> None:
    build_dir = BUILD_DIR / f"{profile.stem}-{build_type.lower()}"
    build_dir.mkdir(parents=True, exist_ok=True)
    conf_args = ""
    if conan_conf:
        conf_args = " ".join(f'-c "{item}"' for item in conan_conf) + " "
    ctx.run(
        (
            f'conan install . -of "{build_dir}" '
            f'-pr:h "{profile}" -pr:b "{profile}" '
            f"{conf_args}"
            f"-s build_type={build_type} --build=missing"
        ),
        echo=True,
    )


@task(iterable=["conan_conf"])
def deps(ctx, profile=None, conan_conf=None):
    # 生成 debug/release 的 Conan 输出（含 toolchain）
    profile_path = Path(profile) if profile else _default_profile()
    _conan_install(ctx, "Debug", profile_path, conan_conf)
    _conan_install(ctx, "Release", profile_path, conan_conf)

@task
def build(ctx, preset=None):
    # 配置并编译 debug/release
    if preset:
        ctx.run(f'cmake --preset "{preset}"', echo=True)
        ctx.run(f'cmake --build --preset "{preset}"', echo=True)
        return

    system = platform.system().lower()
    if system == "windows":
        presets = ["windows-debug", "windows-release"]
    elif system == "linux":
        presets = ["linux-debug", "linux-release"]
    else:
        raise RuntimeError(f"Unsupported system: {system}")

    for name in presets:
        ctx.run(f'cmake --preset "{name}"', echo=True)
        ctx.run(f'cmake --build --preset "{name}"', echo=True)


@task
def clean(ctx):
    # 清理生成目录
    if BUILD_DIR.exists():
        shutil.rmtree(BUILD_DIR)
