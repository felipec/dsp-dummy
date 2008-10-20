	.sect ".3DAC26D0_6D4B_11DD_AD8B_0800200C9A66"
	.string "1024," ; cbstruct (NOT USED);
	.string "3DAC26D0_6D4B_11DD_AD8B_0800200C9A66," ; uuid;
	.string "dummy," ; name;
	.string "1," ; type;

	.string "0," ; (NOT USED);
	.string "1024," ; (NOT USED);
	.string "512," ; (NOT USED);
	.string "128," ; (NOT USED);
	.string "3072," ; (NOT USED);
	.string "5," ; (NOT USED);
	.string "3," ; (NOT USED);
	.string "1000," ; (NOT USED);
	.string "100," ; (NOT USED);
	.string "10," ; (NOT USED);
	.string "1," ; priority;
	.string "1024," ; stack size;
	.string "16," ; system stack size (arbitrary)

	.string "0," ; stack segment;
	.string "3," ; max message depth queued to node;
	.string "1," ; # of input streams;
	.string "1," ; # of output streams;
	.string "3e8H," ; timeout value of GPP blocking calls;

	.string "dummy_create," ; create phase name;
	.string "dummy_execute," ; execute phase name;
	.string "dummy_delete," ; delete phase name;

	.string "0," ; message segment;
	.string "32768," ; (NOT USED);

	.string "none," ; XDAIS algorithm structure name;
	.string "1," ; dynamic loading flag;

	.string "ff3f3f3fH," ; dynamic load data mem seg mask;
	.string "ff3f3f3fH," ; dynamic load code mem seg mask;
	.string "16," ; max # of node profiles supported;
	.string "0," ; node profile 0;
	.string "0," ; node profile 1;
	.string "0," ; node profile 2;
	.string "0," ; node profile 3;
	.string "0," ; node profile 4;
	.string "0," ; node profile 5;
	.string "0," ; node profile 6;
	.string "0," ; node profile 7;
	.string "0," ; node profile 8;
	.string "0," ; node profile 9;
	.string "0," ; node profile 10;
	.string "0," ; node profile 11;
	.string "0," ; node profile 12;
	.string "0," ; node profile 13;
	.string "0," ; node profile 14;
	.string "0," ; node profile 15;
	.string "none," ; stackSegName segment;

	.sect ".dcd_register";
	.string "3DAC26D0_6D4B_11DD_AD8B_0800200C9A66:0,";
