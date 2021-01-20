#!/usr/bin/env Rscript

# install.packages ('ggplot2')
library(ggplot2)
## # install.packages ('scales')
## library (scales)

args = commandArgs(trailingOnly=TRUE)
#data = read.table(args[1], header=T, sep=";")
data = read.table("../otherResult/ieee80211adBeamSweepingTime.csv", header=T, sep=";")
#data = data[c(1:7,8:nrow(data)),]

data
data$Time

data$Algorithm = factor(data$Algorithm, levels = c("100 ms", "60 ms", "30 ms"))
data$Type = factor(data$Type, levels = c("Transmission Time", "Sweeping Time"))
#data$Algorithm = factor(data$Algorithm)
#data$Timeslot =  factor(data$Timeslot, levels = c("1 a.m.","2 a.m.","3 a.m.","4 a.m.","5 a.m.","6 a.m.","7 a.m.","8 a.m.","9 a.m.","10 a.m.","11 a.m.","12 p.m.",
#                                         "1 p.m.","2 p.m.","3 p.m.","4 p.m.","5 p.m.","6 p.m.","7 p.m.","8 p.m.","9 p.m.","10 p.m.","11 p.m.","12 a.m."))
#data$Mean = data$Mean
#data$SD = data$SD
data$SweepingTime = data$SweepingTime
data$TransmissionTime = data$TransmissionTime

dodge <- position_dodge(width=2)
pd <- position_dodge(0.0)

g.all<- ggplot(data, aes(x=Algorithm, y=Time)) +
#  geom_errorbar(aes(ymin=Mean-SD, ymax=Mean+SD), width=.2,
#                position=position_dodge(0.0))+
  geom_col(aes(fill = Type), width = 0.7) +
  geom_text(aes(x = Algorithm, y = Time, label = Time, group = Type),
            position = position_stack(vjust = .5), size = 6, color = "white") +
  scale_fill_manual(values = c("#04B486", "#2E9AFE")) +
# geom_text(aes(y = lab_ypos, label = len, group =supp), color = "white")
#  geom_line(aes(color=Algorithm), size=1) +
#  geom_point(aes(shape=Algorithm),size=3) +
#  scale_x_discrete(breaks=c("1 a.m.", "5 a.m.","9 a.m.","1 p.m.","5 p.m.","9 p.m.")) +
  #scale_y_discrete(limits=data$Algorithm) +
  #scale_fill_brewer(palette="Blues")+
#  scale_color_manual(name  ="Algorithm", 
#                     values=c("#04B486","#2E9AFE", "#DBA901", "#5F5C58","#E95E3F", "#7E6148B2", "#F39B7FB2", "#3C5488B2", "#50105A", "#D2E3F5"))+
#  scale_shape_manual(values=c(15, 16, 17, 18, 19, 8, 9, 11)) +
 # scale_shape_discrete(name  ="Algorithm")+
  ggtitle("IEEE 802.11ad Beam sweeping duration")+
  xlab("Beacon interval duration")+
  ylab("Time (ms)")+
  theme(plot.title = element_text(lineheight=.8, size=20, face="bold", hjust=0.8),
        #axis.title.x = element_blank(),
        axis.title.x = element_text(size=18, vjust=-0.8),
        axis.title.y = element_text(size=18),
        axis.text.x  = element_text(vjust=0.35, size=16),
        axis.text.y  = element_text(vjust=0.35, size=16),
        legend.title= element_blank(),
        legend.background = element_rect(colour = "black"),
        legend.key.size = unit(1, "cm"),
        legend.text = element_text(size = 14))
  # Uncomment this line for setting the position of legend
  # legend.position=c(.2, 0.4))+
  #ggsave(file=args[2], width=8, height=6)
  #ggsave(file="../../../../writting/seemoo-thesis-template/gfx/plots/ieee80211adBeamSweepingTime.pdf", width=8, height=6, plot = g.all, device=cairo_pdf)
  ggsave(file="/home/longpham211/Workspace/thesis/veins_simulator/src/veins-mmwave/presentation/ieee80211adBeamSweepingTime.pdf", width=8, height=6, plot = g.all, device=cairo_pdf)  

  #print(g.all)
 #x = dev.off()
