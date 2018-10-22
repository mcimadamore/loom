/*
 * Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

/*
 * @test
 * @bug 8211901
 * @summary  javadoc generates broken links on deprecated items page
 * @library  ../lib
 * @modules  jdk.javadoc/jdk.javadoc.internal.tool
 * @build    JavadocTester
 * @run main TestOverriddenDeprecatedMethods
 */

public class TestOverriddenDeprecatedMethods extends JavadocTester {

    public static void main(String args[]) throws Exception {
        TestOverriddenDeprecatedMethods tester = new TestOverriddenDeprecatedMethods();
        tester.runTests();
    }

    @Test
    void test() {
        javadoc("-d", "out-deprecated",
                "-sourcepath", testSrc,
                "--override-methods","summary",
                "pkg1");

        checkExit(Exit.OK);

        checkOrder("pkg1/SubClass.html",
                "Method Summary",
                "Methods declared in class&nbsp;pkg1.<a href=\"BaseClass.html\" title=\"class in pkg1\">"
                + "BaseClass</a>",
                "<a href=\"BaseClass.html#func3()\">func3</a>");

        checkOrder("pkg1/SubClass.html",
                "Method Detail",
                "@Deprecated\npublic&nbsp;void&nbsp;func1()",
                "<div class=\"deprecationBlock\"><span class=\"deprecatedLabel\">Deprecated.</span></div>",
                "@Deprecated\npublic&nbsp;void&nbsp;func2()",
                "<div class=\"deprecationBlock\"><span class=\"deprecatedLabel\">Deprecated.</span></div>",
                "<div class=\"block\">deprecated with comments</div>");
    }
}
