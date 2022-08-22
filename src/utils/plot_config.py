# Where shall plots be saved, relative to the root directory of the project.
IMAGES_PATH = "write_up/thesis/images/plots/"
# Size of the seaborn's marker in plots
MARKER_SIZE = 7
# Width of lines in plots
LINE_WIDTH = 0.7

SUPTITLE_Y = 1.11
SUPTITLE_FONT_SIZE = 30
X_LABEL_FONT_SIZE = 15
Y_LABLE_FONT_SIZE = 20
LEGEND_TITLE_FONT_SIZE = 20
LEGEND_TEXT_FONT_SIZE = 15

# centimeters in inches
cm = 1/2.54

PAGE_WIDTH_EXCLUDING_MARGINS = 14.3*cm
FIG_SIZE = (PAGE_WIDTH_EXCLUDING_MARGINS, 9*cm)

# Determines if the plots are being generated for a paper or a poster,
# using different plot settings in either case.
PLOT_CONTEXT = "paper"
# PLOT_CONTEXT = "poster"

# Determines how standard deviation is shown in plots.
# Shown as bars when the context is a paper, as bands if a poster.
err_style = {
    "err_style": "band" if PLOT_CONTEXT == "poster" else "bars",
}

# Make the bars look larger, and black, if the plot context is a paper.
if PLOT_CONTEXT != "poster":
    err_style["err_kws"] = {"linewidth": LINE_WIDTH, "capsize": 2, "ecolor": "k"}

# Pretty plot styles, can be used to standardize the look and feel of plots.
sns_base_kwargs = {
    "markers": True,
    "dashes": True,
    "markeredgecolor": "none",
}

sns_kwargs = {
    **sns_base_kwargs,
    "markersize": MARKER_SIZE,
    "linewidth": LINE_WIDTH,
    "ci": "sd",
    **err_style,
}

# A mapping of numbers to their respective words
num_to_word_dict = {
    0 : "Zero",
    1 : "One",
    2 : "Two",
    3 : "Three",
    4 : "Four",
    5 : "Five",
    6 : "Six",
    7 : "Seven",
    8 : "Eight",
    9 : "Nine",
    10 : "Ten",
    11 : "Eleven",
    12 : "Twelve"
}

# University of Malta's colour pallette.
uom_color_palette = ["#9C0C35", "#E94C5E", "#F08590", "#F5B4C7", "#FAC18A"]

grid_style = "white" if PLOT_CONTEXT == "poster" else "whitegrid"