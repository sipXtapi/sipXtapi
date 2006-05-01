#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'ipaddr'

require File.join(File.dirname(__FILE__), 'exceptions')
require File.join(File.dirname(__FILE__), 'sipx_ipsocket')
require File.join(File.dirname(__FILE__), 'socket_utils')


class Utils
  # Utils has only class methods, so don't allow instantiation.
  private_class_method :new

public

  # Take a contact string like "Jorma Kaukonen"<sip:187@10.1.1.170:1234>;tag=1c32681
  # and extract just the host part, in this case "10.1.1.170".  The "@" is optional,
  # could be just "sip:10.1.1.170" for example.
  # :LATER: Use regex here, much simpler
  def Utils.contact_host(contact)
    # Find the colon at the end of "sip:" or "sips:".  Strip the colon and
    # everything before it.
    colon_index = contact.index(':')
    if !colon_index
      raise(BadContactException.new(contact),
            "Bad contact, can't find colon: \"#{contact}\"",
            caller)
    end
    contact = contact[(colon_index + 1)..-1]
    
    # If there is an @, then remove the part of the contact up to and including
    # the @.
    at_sign_index = contact.index("@")
    if at_sign_index
      contact = contact[at_sign_index + 1..-1]
    end
    
    # If there is a semicolon indicating contact params, then strip the params
    contact = Utils.remove_part_of_str_beginning_with_char(contact, ';')
    
    # If there is an ">" at the end, then remove it
    contact = Utils.remove_part_of_str_beginning_with_char(contact, '>')
    
    # If there is another colon indicating a port #, then remove the port #
    contact = Utils.remove_part_of_str_beginning_with_char(contact, ':')
  end

  def Utils.contact_ip_addr(contact)
    # Extract the host part of the contact
    host = contact_host(contact)

    # Resolve the host to an IP address, if it's a domain name
    if SipxIPSocket.valid_ipaddr?(host)
      host
    else
      SocketUtils.ip_address_from_domain_name(host)
    end
  end

  # Look for the char in the str.  If found, then remove that char and everything
  # after it.  Return the str.
  def Utils.remove_part_of_str_beginning_with_char(str, char)
    char_index = str.index(char)
    if char_index
      str = str[0...char_index]
    end
    str
  end

  # Return just the AOR part of a SIP From or To header, stripping the tag if
  # one is present.
  # If there is no tag then raise BadSipHeaderException if is_tag_required is true.
  # :LATER: Use regex here, much simpler
  def Utils.get_aor_from_header(header, is_tag_required = true)
    aor = nil
    
    # find the semicolon preceding the tag
    semi = header.index(';')
    
    # extract the AOR
    if semi
      aor = header[0, semi]
    else
      if is_tag_required
        raise(BadSipHeaderException. new(header),
              'Tag missing from SIP header',
              caller)
      end
      aor = header
    end
    
    # put '>' at the end to balance '<' at the start, if necessary
    if aor[0] == ?< and aor[-1] != ?>
      aor << '>'
    end
    
    aor
  end

  # Given a contact URL with params, like <sip:101@10.1.20.3:5100;play=https%3A%2F%2Flocalhost>,
  # remove the params, which are preceded by semicolons.  Usually there is a '>' at the end
  # matching a '<' at the beginning.  If so then leave it in place.
  # :LATER: Use regex here, much simpler
  def Utils.contact_without_params(contact)
    semi_index = contact.index(';')
    if semi_index
      gt_index = contact.index('>')
      contact = contact[0...semi_index]
      contact << '>' if gt_index
    end
    contact
  end
  
  # Raise a CallResolverException.  Include the stack trace.
  def Utils.raise_exception(msg, klass = CallResolverException)
    raise(klass, msg, caller)
  end

  # Given an events array, return a string that displays one event per line,
  # with preceding newlines.  Used for debugging.
  def Utils.events_to_s(events)
    events.inject('') {|str, event| str + "\n" + event.to_s}
  end

end
