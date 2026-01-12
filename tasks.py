

from pathlib import Path
import json
import os
import platform
import shlex
import shutil
import subprocess

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

def _default_preset() -> str:
    system = platform.system().lower()
    if system == "windows":
        return "windows-debug"
    if system == "linux":
        return "linux-debug"
    raise RuntimeError(f"Unsupported system: {system}")

def _preset_binary_dir(preset_name: str) -> Path:
    presets_path = ROOT / "CMakePresets.json"
    data = json.loads(presets_path.read_text(encoding="utf-8"))
    for preset in data.get("configurePresets", []):
        if preset.get("name") == preset_name:
            binary_dir = preset.get("binaryDir")
            if not binary_dir:
                raise RuntimeError(f"Preset missing binaryDir: {preset_name}")
            return Path(binary_dir.replace("${sourceDir}", str(ROOT)))
    raise RuntimeError(f"Preset not found: {preset_name}")

def _write_clang_uml_config(config_path: Path, compile_dir: Path, output_dir: Path) -> None:
    compile_dir_posix = compile_dir.as_posix()
    output_dir_posix = output_dir.as_posix()
    include_dir_posix = (ROOT / "include").as_posix()
    src_dir_posix = (ROOT / "src").as_posix()
    plugins_dir_posix = (ROOT / "plugins").as_posix()
    config_text = (
        f'compilation_database_dir: "{compile_dir_posix}"\n'
        f'output_directory: "{output_dir_posix}"\n'
        "diagrams:\n"
        "  gtools_class:\n"
        "    type: class\n"
        "    glob:\n"
        f'      - "{include_dir_posix}/**/*.h"\n'
        f'      - "{include_dir_posix}/**/*.hpp"\n'
        f'      - "{src_dir_posix}/**/*.h"\n'
        f'      - "{src_dir_posix}/**/*.hpp"\n'
        f'      - "{src_dir_posix}/**/*.c"\n'
        f'      - "{src_dir_posix}/**/*.cc"\n'
        f'      - "{src_dir_posix}/**/*.cxx"\n'
        f'      - "{src_dir_posix}/**/*.cpp"\n'
        f'      - "{plugins_dir_posix}/**/*.h"\n'
        f'      - "{plugins_dir_posix}/**/*.hpp"\n'
        f'      - "{plugins_dir_posix}/**/*.c"\n'
        f'      - "{plugins_dir_posix}/**/*.cc"\n'
        f'      - "{plugins_dir_posix}/**/*.cxx"\n'
        f'      - "{plugins_dir_posix}/**/*.cpp"\n'
    )
    config_path.write_text(config_text, encoding="utf-8")


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

@task
def class_diagram(ctx, preset=None, output_dir=None):
    """
    Generate a class diagram using clang-uml and the selected CMake preset.
    """
    preset_name = preset or _default_preset()
    build_dir = _preset_binary_dir(preset_name)
    compile_commands = build_dir / "compile_commands.json"
    if not compile_commands.exists():
        raise RuntimeError(
            f"compile_commands.json not found in {build_dir}. "
            "Run `invoke build --preset <preset>` first."
        )

    output_path = Path(output_dir) if output_dir else (BUILD_DIR / "uml" / preset_name)
    output_path.mkdir(parents=True, exist_ok=True)
    config_path = output_path / "clang-uml.yml"
    _write_clang_uml_config(config_path, build_dir, output_path)

    ctx.run(f'clang-uml -c "{config_path}" -g mermaid -n gtools_class', echo=True)
