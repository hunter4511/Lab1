awk  ' {url[$12] += 1; s+=1;}
END 	{
for(i in url) {print i, "-", url[i], "-", url[i]*100/s, "%" }
	}' "log.txt" | sort -nrk3 | head 
