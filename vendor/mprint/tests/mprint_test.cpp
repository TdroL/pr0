#ifdef TEST

#include "../include/mprint.hpp"

#include <iostream>
#include <vector>
#include <string>

int main()
{

	int i = -1;
	unsigned int ui = 10;
	float f = 1.5f;
	double d = 3.141596;
	char c = 'a';
	char *c_str = "c_str";
	std::string str = "str";

	std::vector<int> vi{-1, -2, -3};
	std::vector<unsigned int> vui{1, 2, 3};
	std::vector<float> vf{0.5f, 0.25f, 0.125f};
	std::vector<double> vd{0.0001, 0.00001, 0.000001, 0.0000001};
	std::vector<char> vc{'h', 'e', 'l', 'l', 'o'};
	std::vector<char *> vc_str{"c_str1", "c_str2", "c_str3", "c_str4ext"};
	std::vector<std::string> vstr{"str1", "str2", "str3", "str4ext"};

	std::cout << "Test \"print\" (sep, new line):" << std::endl;
	print "i =", i;
	print "ui =", ui;
	print "f =", f;
	print "d =", d;
	print "c =", c;
	print "c_str =", c_str;
	print "str =", str;
	print "vi =", vi;
	print "vui =", vui;
	print "vf =", vf;
	print "vd =", vd;
	print "vc =", vc;
	print "vc_str =", vc_str;
	print "vstr =", vstr;

	std::cout << "----------------------------------" << std::endl;
	std::cout << "Test \"sputs\" (sep, no new line):" << std::endl;
	puts "i =", i, "\n";
	puts "ui =", ui, "\n";
	puts "f =", f, "\n";
	puts "d =", d, "\n";
	puts "c =", c, "\n";
	puts "c_str =", c_str, "\n";
	puts "str =", str, "\n";
	puts "vi =", vi, "\n";
	puts "vui =", vui, "\n";
	puts "vf =", vf, "\n";
	puts "vd =", vd, "\n";
	puts "vc =", vc, "\n";
	puts "vc_str =", vc_str, "\n";
	puts "vstr =", vstr, "\n";

	std::cout << "----------------------------------" << std::endl;
	std::cout << "Test \"echo\" (no sep, no new line):" << std::endl;
	echo "i = ", i, "\n";
	echo "ui = ", ui, "\n";
	echo "f = ", f, "\n";
	echo "d = ", d, "\n";
	echo "c = ", c, "\n";
	echo "c_str = ", c_str, "\n";
	echo "str = ", str, "\n";
	echo "vi = ", vi, "\n";
	echo "vui = ", vui, "\n";
	echo "vf = ", vf, "\n";
	echo "vd = ", vd, "\n";
	echo "vc = ", vc, "\n";
	echo "vc_str = ", vc_str, "\n";
	echo "vstr = ", vstr, "\n";

	std::cout << "----------------------------------" << std::endl;
	std::cout << "Test \"echon\" (no sep, new line):" << std::endl;
	echon "i = ", i;
	echon "ui = ", ui;
	echon "f = ", f;
	echon "d = ", d;
	echon "c = ", c;
	echon "c_str = ", c_str;
	echon "str = ", str;
	echon "vi = ", vi;
	echon "vui = ", vui;
	echon "vf = ", vf;
	echon "vd = ", vd;
	echon "vc = ", vc;
	echon "vc_str = ", vc_str;
	echon "vstr = ", vstr;

	std::cout << "----------------------------------" << std::endl;
	std::cout << "Test \"puts\" collision:" << std::endl;
	puts "puts\n";
	puts("puts()\n");

	std::cout << "OK" << std::endl;

	return 0;
}

#endif