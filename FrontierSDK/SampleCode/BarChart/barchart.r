#include <Types.r>
#include <SysTypes.r>

resource 'vers' (1) {
	0x4,
	0x10,
	beta,
	0x1,
	verUS,
	"4.1b1",
	"4.1b1; © 1991-1996, UserLand Software, Inc."
};

resource 'vers' (2) {
	0x4,
	0x10,
	beta,
	0x1,
	verUS,
	"4.1b1",
	"Graphics for Scripting"
};

resource 'STR#' (129, "Defaults", purgeable) {
	{	/* array StringArray: 6 elements */
		/* [1] */
		"BarChart",
		/* [2] */
		"Demo application for UserLand Frontier scripting",
		/* [3] */
		"Property of UserLand Software, Inc.",
		/* [4] */
		"http://www.scripting.com/frontier",
		/* [5] */
		"© 1991-96 UserLand Software",
		/* [6] */
		"Version 4.1; September 1996"
	}
};
