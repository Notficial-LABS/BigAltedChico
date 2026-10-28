// SPDX-License-Identifier: MIT
// Run from the repository root: swift Tools/render_macos_welcome.swift
// Native RTFD avoids Installer's incomplete HTML/CSS image sizing.
import AppKit
let root = URL(fileURLWithPath: FileManager.default.currentDirectoryPath)
let image = NSImage(contentsOf: root.appendingPathComponent("Assets/Installer/Banner.png"))!
image.size = NSSize(width: 394, height: 60.16)
for representation in image.representations { representation.size = image.size }
let imageFile = FileWrapper(regularFileWithContents: image.tiffRepresentation!)
imageFile.preferredFilename = "Banner.tiff"
let attachment = NSTextAttachment(fileWrapper: imageFile)
attachment.attachmentCell = NSTextAttachmentCell(imageCell: image)
let document = NSMutableAttributedString(attachment: attachment)
let style = NSMutableParagraphStyle()
style.paragraphSpacing = 9
func append(_ value: String, size: CGFloat = 12, bold: Bool = false) {
    document.append(NSAttributedString(string: value, attributes: [
        .font: bold ? NSFont.boldSystemFont(ofSize: size) : NSFont.systemFont(ofSize: size),
        .paragraphStyle: style
    ]))
}
append("\nBigAltedChico 0.1\n", size: 18, bold: true)
append("Choose AU, VST3 and/or Standalone in Installation Type.\n")
append("All users: /Applications. Only me: ~/Applications. Choose in Destination Select.\n")
append("Close your DAW and BigAltedChico before installing.\n")
append("Presets, skins and preferences are preserved.\n")
append("Notficial LABS · macOS Apple Silicon\nLocal test build · Not signed or notarized.", size: 10)
let range = NSRange(location: 0, length: document.length)
let wrapper = try document.fileWrapper(from: range, documentAttributes: [.documentType: NSAttributedString.DocumentType.rtfd])
let output = root.appendingPathComponent("Packaging/macOS/welcome.rtfd")
try wrapper.write(to: output, options: .atomic, originalContentsURL: nil)
// Verify the serialized attachment and produce a native text-layout preview.
let restored = try NSAttributedString(url: output, options: [.documentType: NSAttributedString.DocumentType.rtfd], documentAttributes: nil)
let storage = NSTextStorage(attributedString: restored)
let layout = NSLayoutManager()
let container = NSTextContainer(containerSize: NSSize(width: 410, height: 1000))
storage.addLayoutManager(layout); layout.addTextContainer(container)
layout.ensureLayout(for: container)
let bounds = layout.usedRect(for: container)
precondition(bounds.width <= 410 && bounds.height < 320, "Welcome must fit the native Installer content area")
restored.enumerateAttribute(.attachment, in: NSRange(location: 0, length: restored.length)) { value, _, _ in
    if let a = value as? NSTextAttachment {
        precondition((a.attachmentCell?.cellSize().width ?? 1000) <= 394.1)
        print("Serialized banner:", a.attachmentCell!.cellSize())
    }
}
let preview = NSImage(size: NSSize(width: 410, height: 320))
preview.lockFocusFlipped(true)
NSColor.white.setFill(); NSRect(origin: .zero, size: preview.size).fill()
layout.drawGlyphs(forGlyphRange: layout.glyphRange(for: container), at: NSPoint(x: 0, y: 4))
preview.unlockFocus()
let bitmap = NSBitmapImageRep(data: preview.tiffRepresentation!)!
let previewDir = root.appendingPathComponent("Working/Previews")
try FileManager.default.createDirectory(at: previewDir, withIntermediateDirectories: true)
try bitmap.representation(using: .png, properties: [:])!.write(to: previewDir.appendingPathComponent("macos-installer-welcome.png"))
print("Welcome layout:", bounds)
