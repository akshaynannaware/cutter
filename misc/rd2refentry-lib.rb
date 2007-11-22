require "erb"
require "rd/rdvisitor"
require "rd/version"

module RD
  class RD2RefEntryVisitor < RDVisitor
    include ERB::Util

    SYSTEM_NAME = "RDtool -- RD2RefEntryVisitor"
    SYSTEM_VERSION = "0.0.1"
    VERSION = Version.new_from_version_string(SYSTEM_NAME, SYSTEM_VERSION)

    def self.version
      VERSION
    end

    # must-have constants
    OUTPUT_SUFFIX = "xml"
    INCLUDE_SUFFIX = ["xml"]

    METACHAR = {"<" => "&lt;", ">" => "&gt;", "&" => "&amp;"}

    def initialize
      super
    end

    def apply_to_DocumentElement(element, contents)
      ret = ""
      ret << "#{xml_decl}\n"
      ret << "#{doctype_decl}\n"
      ret << "<refentry id=\"#{ref_entry_id}\">\n"
      ret << "#{ref_meta}\n"
      ret << "#{ref_name_div}\n"
      ret << collect_section_contents(contents).join("\n\n")
      ret << "</refentry>\n"
      ret
    end

    def apply_to_Headline(element, title)
      if element.level == 1
        @title, @purpose = title.join.split(/\B---\B/, 2)
      end
      [:headline, element.level - 1, title]
    end

    def apply_to_TextBlock(element, contents)
      tag("para", {}, *contents)
    end

    def apply_to_Verbatim(element)
      contents = []
      element.each_line do |line|
	contents.push(apply_to_String(line))
      end
      tag("programlisting", {}, contents.join("").chomp)
    end

    def apply_to_StringElement(element)
      apply_to_String(element.content.chomp)
    end

    def apply_to_String(element)
      h(element)
    end

    private
    def xml_decl
      "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
    end

    def doctype_decl
      "<!DOCTYPE refentry \n" +
        "  PUBLIC \"-//OASIS//DTD DocBook XML V4.1.2//EN\"\n" +
        "  \"http://www.oasis-open.org/docbook/xml/4.1.2/docbookx.dtd\">"
    end

    def ref_entry_id
      File.basename(ARGF.filename, ".*").downcase
    end

    def tag(name, attributes={}, *contents)
      encoded_attributes = attributes.collect do |key, value|
        "#{key}=\"#{h(value)}\""
      end.join(" ")
      unless encoded_attributes.empty?
        encoded_attributes = " #{encoded_attributes}"
      end

      if contents.size == 1 and contents[0] !~ /</
        indented_contents = contents[0]
      else
        indented_contents = contents.collect do |content|
          "  #{content}"
        end.join("\n")
        indented_contents = "\n#{indented_contents}\n"
      end

      if indented_contents.empty?
        "<#{name}#{encoded_attributes}/>"
      else
        "<#{name}#{encoded_attributes}>#{indented_contents}</#{name}>"
      end
    end

    def ref_meta
      tag("refmeta", {},
          tag("refentrytitle", {}, @title),
          tag("refmiscinfo", {}, h("Cutter Library")))
    end

    def ref_name_div
      tag("refnamediv", {},
          tag("refname", {}, @title),
          tag("refpurpose", {}, @purpose))
    end

    def collect_section_contents(contents)
      section_contents = pre_collect_section_contents(contents)
      post_collect_section_contents(section_contents)
    end

    def pre_collect_section_contents(contents)
      section_contents = [[0, []]]
      contents.each do |content|
        if content.is_a?(Array) and content[0] == :headline
          _, level, title = content
          title_tag = tag("title", {}, *title)
          sub_section_contents = []
          while section_contents.last[0] > level
            sub_level, *sub_contents = section_contents.pop
            sub_section_contents.unshift(tag("refsect#{sub_level}", {},
                                             *sub_contents.flatten) + "\n")
          end
          section_contents.last[1].concat(sub_section_contents)
          section_contents << [level, [title_tag]]
        else
          section_contents.last[1] << content
        end
      end
      section_contents
    end

    def post_collect_section_contents(section_contents)
      collected_section_contents = []
      section_contents.last[0].downto(0) do |level|
        sub_section_contents = []
        while section_contents.last[0] > level
          sub_level, sub_contents = section_contents.pop
          sub_section_contents.unshift(tag("refsect#{sub_level}", {},
                                           *sub_contents))
        end
        unless sub_section_contents.empty?
          raise "!?" unless section_contents.last[0] == level
          _, contents = section_contents.pop
          tagged_contents = tag("refsect#{level}", {},
                                *(contents + sub_section_contents))
          if level > 0
            contents = tagged_contents
          else
            contents = sub_section_contents.join("\n\n")
          end
          collected_section_contents.unshift(contents + "\n")
        end
      end
      collected_section_contents
    end

    def apply_to_ItemList(element, items)
      tag("itemizedlist", {}, *items)
    end

    def apply_to_EnumList(element, items)
      tag("orderedlist", {}, *items)
    end

    def apply_to_ItemListItem(element, contents)
      tag("listitem", {}, *contents)
    end

    def apply_to_EnumListItem(element, content)
      tag("listitem", {}, *contents)
    end

    def consist_of_one_textblock?(listitem)
      listitem.children.size == 1 and listitem.children[0].is_a?(TextBlock)
    end
  end
end

$Visitor_Class = RD::RD2RefEntryVisitor
