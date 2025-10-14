import pytest
from contextlib import nullcontext
import esphome.config_validation as cv
from .sensor import calc_spad_xy, check_roi_center

@pytest.mark.parametrize(
    "x, y, index",
    [
        (0, 15, 128),
        (7, 15, 184),
        (8, 15, 192),
        (15, 15, 248),
        (0, 8, 135),
        (7, 8, 191),
        (8, 8, 199),
        (15, 8, 255),
        (0, 7, 127),
        (7, 7, 71),
        (8, 7, 63),
        (15, 7, 7),
        (0, 0, 120),
        (7, 0, 64),
        (8, 0, 56),
        (15, 0, 0),
    ],
)
def test_calc_spad_xy(x, y, index):
    """Test that calc_spad_xy() returns the correct x/y for specified SPAD index."""
    result = calc_spad_xy(index)
    assert result["x"] == x
    assert result["y"] == y


@pytest.mark.parametrize('x', range(0, 16))
@pytest.mark.parametrize('y', range(0, 16))
def test_check_roi_center(x, y):
    """Test that check_roi_center() throws error for invalid ROI"""
    obj = {"roi_center": {"x": x, "y": y}, "roi_size": {"x":4, "y":5}}
    x_exp = pytest.raises(cv.Invalid) if x<2 or x>14 else nullcontext({"x": x})
    y_exp = pytest.raises(cv.Invalid) if y<2 or y>13 else nullcontext({"y": y})
    with x_exp as e:
        assert check_roi_center("x", obj) == obj
    with y_exp as e:
        assert check_roi_center("y", obj) == obj