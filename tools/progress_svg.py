#!/usr/bin/env python3
"""Draw the README's progress badges and unit chart from a build progress report.

Run from the repository root after building a version, for example:

    uv run --locked python tools/progress_svg.py build/GUPE8P/report.json

The report is the one the build writes (ninja build/GUPE8P/report.json). This writes
docs/progress/code.svg and docs/progress/data.svg (badges of the matched code and data
percentages) and docs/progress/chart.svg (a map of the version's units, each sized by its
code and shaded by how much of it matches). The output depends only on the report and the
commit it describes, so the same inputs always give the same files. To update the README's
images for merged main, build main, run this with --commit set to main's commit, and commit
the three files through a pull request.
"""

import argparse
import json
from pathlib import Path
import subprocess
from typing import List, NamedTuple, Tuple
from xml.sax.saxutils import escape

PROJECT_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_OUTPUT = PROJECT_ROOT / "docs" / "progress"

# The chart's size, which the README shows it at
CHART_WIDTH = 512
CHART_HEIGHT = 256

# Shading from no code matching to all of it matching
COLOR_UNMATCHED = (0x3A, 0x3F, 0x4B)
COLOR_MATCHED = (0x2E, 0xA0, 0x43)
COLOR_BACKGROUND = "#1F2329"
COLOR_TEXT = "#FFFFFF"
COLOR_BADGE_LABEL = "#555555"
COLOR_BADGE_VALUE = "#1F6FB2"

# Badge geometry, sized for Verdana 11px text
BADGE_HEIGHT = 20
BADGE_CHAR_WIDTH = 7
BADGE_PADDING = 6


class Unit(NamedTuple):
    """A unit of the chart: its name, its code bytes and the share of them that matches."""

    name: str
    size: int
    matched_percent: float


class Rect(NamedTuple):
    x: float
    y: float
    width: float
    height: float


def load_report(path: Path) -> dict:
    with path.open(encoding="utf-8") as file:
        return json.load(file)


def git_commit() -> str:
    """The short commit of the working tree, for the files' provenance."""
    result = subprocess.run(
        ["git", "rev-parse", "--short", "HEAD"],
        cwd=PROJECT_ROOT,
        capture_output=True,
        text=True,
        check=True,
    )
    return result.stdout.strip()


def percent_text(value: float) -> str:
    return f"{value:.2f}%"


def chart_units(report: dict) -> List[Unit]:
    """The units with code, largest first; equal sizes are ordered by name."""
    units = []
    for unit in report["units"]:
        measures = unit.get("measures", {})
        size = int(measures.get("total_code", 0))
        if size == 0:
            continue
        matched = float(measures.get("matched_code_percent", 0.0))
        units.append(Unit(unit["name"], size, matched))
    units.sort(key=lambda unit: (-unit.size, unit.name))
    return units


def worst_aspect(row: List[float], side: float) -> float:
    """The worst aspect ratio of a row of areas laid along a side of the given length."""
    total = sum(row)
    thickness = total / side
    worst = 1.0
    for area in row:
        length = area / thickness
        aspect = max(length / thickness, thickness / length)
        worst = max(worst, aspect)
    return worst


def squarify(areas: List[float], rect: Rect) -> List[Rect]:
    """Lays out areas (largest first) in a rectangle, keeping each piece close to square.

    Each row runs along the rectangle's shorter side; a row grows while that makes its worst
    piece more square, then the rest is laid out in the space left.
    """
    rects: List[Rect] = []
    index = 0
    x, y, width, height = rect
    while index < len(areas):
        side = min(width, height)
        row = [areas[index]]
        index += 1
        while index < len(areas):
            candidate = row + [areas[index]]
            if worst_aspect(candidate, side) > worst_aspect(row, side):
                break
            row = candidate
            index += 1

        thickness = sum(row) / side
        offset = 0.0
        for area in row:
            length = area / thickness
            if width >= height:
                rects.append(Rect(x, y + offset, thickness, length))
            else:
                rects.append(Rect(x + offset, y, length, thickness))
            offset += length
        if width >= height:
            x += thickness
            width -= thickness
        else:
            y += thickness
            height -= thickness
    return rects


def shade(matched_percent: float) -> str:
    """The color for a unit, from unmatched to matched."""
    fraction = max(0.0, min(1.0, matched_percent / 100.0))
    channels: Tuple[int, ...] = tuple(
        round(low + (high - low) * fraction)
        for low, high in zip(COLOR_UNMATCHED, COLOR_MATCHED)
    )
    return "#{:02X}{:02X}{:02X}".format(*channels)


def svg_number(value: float) -> str:
    """A coordinate with at most two decimals, without trailing zeros."""
    text = f"{value:.2f}".rstrip("0").rstrip(".")
    return text if text else "0"


def chart_svg(report: dict, version: str, commit: str) -> str:
    """A map of the version's units, sized by code and shaded by matched code."""
    measures = report["measures"]
    units = chart_units(report)
    total = sum(unit.size for unit in units)
    scale = CHART_WIDTH * CHART_HEIGHT / total
    areas = [unit.size * scale for unit in units]
    rects = squarify(areas, Rect(0.0, 0.0, CHART_WIDTH, CHART_HEIGHT))

    title = f"{version} code progress: {percent_text(measures['matched_code_percent'])} matched"
    desc = (
        f"{measures['matched_code']} of {measures['total_code']} code bytes "
        f"({measures['matched_functions']} of {measures['total_functions']} functions) and "
        f"{measures['matched_data']} of {measures['total_data']} data bytes match. Each "
        f"rectangle is one of {len(units)} units with code, sized by its code bytes and "
        f"shaded from grey (none matching) to green (all matching). Generated by "
        f"tools/progress_svg.py from the {version} build report at commit {commit}."
    )

    lines = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{CHART_WIDTH}" '
        f'height="{CHART_HEIGHT}" viewBox="0 0 {CHART_WIDTH} {CHART_HEIGHT}" role="img" '
        f'aria-labelledby="title desc">',
        f'<title id="title">{escape(title)}</title>',
        f'<desc id="desc">{escape(desc)}</desc>',
        f'<rect width="{CHART_WIDTH}" height="{CHART_HEIGHT}" fill="{COLOR_BACKGROUND}"/>',
        '<g stroke="#1F2329" stroke-width="0.5">',
    ]
    for unit, rect in zip(units, rects):
        lines.append(
            f'<rect x="{svg_number(rect.x)}" y="{svg_number(rect.y)}" '
            f'width="{svg_number(rect.width)}" height="{svg_number(rect.height)}" '
            f'fill="{shade(unit.matched_percent)}"><title>{escape(unit.name)}: '
            f"{percent_text(unit.matched_percent)} of {unit.size} bytes</title></rect>"
        )
    lines.append("</g>")
    lines.append("</svg>")
    return "\n".join(lines) + "\n"


def badge_svg(label: str, value: str, description: str) -> str:
    """A two-part badge: the label on grey, the value on blue."""
    label_width = len(label) * BADGE_CHAR_WIDTH + 2 * BADGE_PADDING
    value_width = len(value) * BADGE_CHAR_WIDTH + 2 * BADGE_PADDING
    width = label_width + value_width
    title = f"{label}: {value}"
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{BADGE_HEIGHT}" '
        f'viewBox="0 0 {width} {BADGE_HEIGHT}" role="img" aria-labelledby="title desc">\n'
        f'<title id="title">{escape(title)}</title>\n'
        f'<desc id="desc">{escape(description)}</desc>\n'
        f'<rect width="{label_width}" height="{BADGE_HEIGHT}" rx="3" '
        f'fill="{COLOR_BADGE_LABEL}"/>\n'
        f'<rect x="{label_width}" width="{value_width}" height="{BADGE_HEIGHT}" rx="3" '
        f'fill="{COLOR_BADGE_VALUE}"/>\n'
        f'<g fill="{COLOR_TEXT}" font-family="Verdana,DejaVu Sans,sans-serif" font-size="11" '
        f'text-anchor="middle">\n'
        f'<text x="{label_width / 2:g}" y="14">{escape(label)}</text>\n'
        f'<text x="{label_width + value_width / 2:g}" y="14">{escape(value)}</text>\n'
        f"</g>\n"
        f"</svg>\n"
    )


def write_file(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="\n") as file:
        file.write(text)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("report", type=Path, help="build/<version>/report.json")
    parser.add_argument(
        "--version", help="the version's name (default: the report's folder)"
    )
    parser.add_argument(
        "--commit", help="the commit the report describes (default: HEAD)"
    )
    parser.add_argument(
        "--output", type=Path, default=DEFAULT_OUTPUT, help="output folder"
    )
    args = parser.parse_args()

    report = load_report(args.report)
    version = args.version or args.report.resolve().parent.name
    commit = args.commit or git_commit()
    measures = report["measures"]
    provenance = f"from the {version} build report at commit {commit}"

    code = percent_text(measures["matched_code_percent"])
    data = percent_text(measures["matched_data_percent"])
    write_file(
        args.output / "code.svg",
        badge_svg(
            "Code",
            code,
            f"{measures['matched_code']} of {measures['total_code']} code bytes match, "
            f"{provenance}.",
        ),
    )
    write_file(
        args.output / "data.svg",
        badge_svg(
            "Data",
            data,
            f"{measures['matched_data']} of {measures['total_data']} data bytes match, "
            f"{provenance}.",
        ),
    )
    write_file(args.output / "chart.svg", chart_svg(report, version, commit))


if __name__ == "__main__":
    main()
