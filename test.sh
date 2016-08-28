for i in $( ls market_data_tests/ | grep txt); do
   	echo item: $i
   	./a.out market_data_tests/$i
done
