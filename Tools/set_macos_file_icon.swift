// SPDX-License-Identifier: MIT
// File metadata only: attach a custom Finder icon without changing package bytes.
import AppKit
import Foundation
guard CommandLine.arguments.count == 3,
      let image = NSImage(contentsOfFile: CommandLine.arguments[1]),
      NSWorkspace.shared.setIcon(image, forFile: CommandLine.arguments[2], options: []) else {
    fputs("Unable to set the custom file icon\n", stderr)
    exit(1)
}
