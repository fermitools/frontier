library("grid")
library("lattice")

# Read the data sets, and trim them

#a = read.table("cdf_squidcache_test_20040708.dat", header=T)
squiddat = subset(a,table=="SvxBeamPosition",status=="old",select=c(start.time,dur,records,table,status,swap,memory,cpu.5min,cpu.60min,entries,disk.objects))
#squiddat = subset(a,table=="SvxBeamPosition",status=="new",select=c(start_time,dur,records,table,status,entries))
squiddat$t<-squiddat$dur/1000.
squiddat$time<-squiddat$start.time/1000.
squiddat$gb<-squiddat$swap/1000000
png(filename="squidcache_control_svxbeamposition.png",width = 480,height = 320)
plot(t ~ time,data=squiddat,col="green",xlab="Time (s)", ylab="Access Time (s)",main="Access time for SvxBeamPosition as function of time. Squid cache test") 
dev.off()
