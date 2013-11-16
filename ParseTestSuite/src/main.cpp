//
//  main.cpp
//  ParseTestSuite
//
//  Created by Christian Noon on 11/12/13.
//  Copyright (c) 2013 Christian Noon. All rights reserved.
//

#include "TestRunner.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	parse::TestRunner testRunner(argc, argv);
	int exitCode = app.exec();

	return exitCode;
}
