module States
  
  class ItemOutOfOrderError < StandardError
    def initialize(item)
      super("Item out of order: #{item}")
    end
  end
  
  class History
    # keep at most 30 minutes history
    DEFAULT_MEMORY = 30 * 60;
    
    def initialize(memory = DEFAULT_MEMORY)
      @memory = memory
      @history = []
    end
    
    def add(item, now)
      if !@history.empty?    
        raise ItemOutOfOrderError.new(item) if item.time < @history.last.time              
        trim(now)
      end        
      @history << item
    end
    
    def get_history(from_time)
      i = newer_index(from_time)
      @history.slice(i..@history.size)
    end    
    
    def trim(now)
      from_time = Time.at(now - @memory)
      i = newer_index(from_time)
      @history.slice!(0,  i)
    end
    
    private 
    def newer_index(from_time)
      @history.each_index do |i|
puts "considering #{@history[i].time.rfc2822} >= #{from_time.rfc2822} : #{@history[i].time >= from_time}"
        return i if @history[i].time >= from_time
      end
      return @history.size
    end
  end
end