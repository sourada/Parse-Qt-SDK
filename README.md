Parse-Qt-SDK
============

The Parse Qt SDK is a [FreeBSD](http://en.wikipedia.org/wiki/BSD_licenses) licensed C++ library designed to make working with [Parse](https://www.parse.com) and [Qt 5](http://qt-project.org) as easy as possible. It was designed to closely mimic the iOS SDK to make switching between the two as easy as swapping blocks for signals/slots. It abstracts the entire Rest API from the user by handling all the network requests, replies and JSON parsing in the background.

To ensure the quality of the library, it comes with a Parse Test Suite that has full coverage of the public "User APIs". It is quite easy to invoke. See the Testing section for more details. It is important to note that right now I've only compiled against Mac OS X Mavericks. I will be adding support for Windows here shortly (should be very minor changes).

For more information about the project, updates, pending tickets and in-depth documentation, please refer to the following:

* Parse-Qt-SDK [Home](https://github.com/cnoon/Parse-Qt-SDK)
* Parse-Qt-SDK [Wiki](https://github.com/cnoon/Parse-Qt-SDK/wiki)
* [Parse Documentation](https://parse.com/docs/ios_guide#top/iOS)
* [Qt Documentation](http://qt-project.org/doc/)

Additional Features
-------------------

For anyone that's used the Parse iOS SDK or the REST API, you'll notice the library is not complete. I've almost got everything completed that I'm going to need to use. If anyone else would like to help me expand the functionality, please open some tickets and let's get started!

Testing
-------

The Parse-Qt-SDK uses the QTest library to provide a comprehensive set of unittest suites. To execute all the unittests, follow these instructions:

	$ cd $PARSE_QT_SDK_HOME/ParseTestSuite
	$ qmake -spec macx-xcode (or whatever flavor you like)
	$ Open Xcode project and Build/Run

Support
-------

If you wish to report a bug or request a feature enhancement, please feel free to contact me at <https://github.com/cnoon/Parse-Qt-SDK>.
