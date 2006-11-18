#!/usr/bin/env ruby
DIR = File.dirname(__FILE__)
$:.unshift File.join(DIR, "..")
require 'test/unit'
load 'sipxcar'


class SipxConfigArchiveReaderTest < Test::Unit::TestCase

  def setup
    @car = SipxConfigArchiveReader.new(DIR, '.tgz', DIR + '/')
    @car.verbose = true
  end
    
  def test_get_archives    
    assert_equal ["#{DIR}/a.tgz", "#{DIR}/b.tgz"], @car.get_archives(["a", "b"])
  end

  def test_get_archive_file_listings
    actual = @car.get_archive_file_listings("#{DIR}/birds.tgz")
    expected = ["birds/goose.txt", "birds/sparrow.txt"]
    assert_equal expected, actual
  end
  
  def test_get_archive_file_contents
    actual = @car.get_archive_file_contents("#{DIR}/birds.tgz", "birds/sparrow.txt")
    assert_equal "chirp\n", actual
  end
  
  def test_no_file_differences
    assert @car.get_file_differences("#{DIR}/same1.txt", "#{DIR}/same2.txt").empty?
  end
  
  def test_file_differences
    assert !@car.get_file_differences("#{DIR}/same1.txt", "#{DIR}/different.txt").empty?
  end
  
  def test_get_directory_listing
    assert_equal ["#{DIR}/birds.tgz"], @car.get_directory_listing
  end
  
  def test_packages
    assert_equal ["birds"], @car.get_packages
  end
  
  def test_get_files
      expected = ["birds/goose.txt", "birds/sparrow.txt"]
      assert_equal expected, @car.get_files([])
  end
  
  def test_get_differences
      assert @car.get_differences("#{DIR}/birds.tgz", "birds/sparrow.txt").empty?
      assert_nil @car.get_differences("#{DIR}/birds.tgz", "birds/non-existant")
  end
  
  def test_diff
      @car.diff
  end
  
  def test_reset
      @car.reset
  end
  
  def test_list
      @car.list
  end
  
  def test_modified?
      assert_nil @car.modified_line("#{DIR}/birds.tgz", "birds/sparrow.txt")
      assert_equal "? #{DIR}/birds/non-existant.txt", @car.modified_line("#{DIR}/birds.tgz", "birds/non-existant.txt")
  end
  
  def test_modified 
      @car.modified
  end

  def test_apply
      @car.apply
  end

end
