#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <string.h>



class Layer1_packet{
    public:
        void print_L1_header(void){
            printf("Timestamp = %ld.%06ld Length = %d\n"
            ,(header_.ts).tv_sec, (header_.ts).tv_usec, header_.caplen);
        }
    protected:
        const u_char *packet_ = NULL;
        pcap_pkthdr header_ ;
};

class Layer2_packet :public Layer1_packet{
    public:
        int ETHER_HEAD_LEN = sizeof(struct ether_header);

        void print_ether_header(void){
            set_ether_header();
            printf("SrcMAC: ");print_mac_addr(eth_h_->ether_shost);
            printf("  ");
            printf("DstMAC: ");print_mac_addr(eth_h_->ether_dhost);
            printf("\n");
            print_L3_packet_type(ntohs(eth_h_->ether_type));
	        printf(" ");
        }

        bool is_IPv4(void){
            set_ether_header();
            if(ntohs(eth_h_->ether_type) == 0x0800)return true;
            return false;
        }

        void set_ether_header(void){
            eth_h_ = (struct ether_header*)packet_;
        }

    private:
        struct ether_header *eth_h_ ;

        void print_mac_addr(u_char *ma){
            printf("%02x:%02x:%02x:%02x:%02x:%02x",
                    ma[0],ma[1],ma[2],ma[3],ma[4],ma[5]);
        }

        void print_L3_packet_type(u_short type){ 
            switch(type){
                case 0x0800:
                    printf("IPv4");
                    break;
                case 0x86DD:
                    printf("IPv6");
                    break;
                case 0x0806:
                    printf("ARP");
                    break;
                default:
                    printf("Unknown type");
            }
        }
};

class Layer3_packet :public Layer2_packet{
    public:
        int IPv4_HDR_LEN = sizeof(struct iphdr);
	    void print_ip_header(void){
	        set_ip_header();

	        char saddrstr[64], daddrstr[64];
	        inet_ntop(AF_INET, &ip_header_->saddr, saddrstr, sizeof(saddrstr));
            inet_ntop(AF_INET, &ip_header_->daddr, daddrstr, sizeof(daddrstr));

	        printf("SrcIP: "); printf("%s ",saddrstr);
 	        printf("DstIP: "); printf("%s\n",daddrstr);
	        print_L4_packet_type(ip_header_->protocol);
	        printf(" ");
	    }

	    bool is_tcp(void){
            set_ip_header();
            if(is_IPv4()){
	            if(ip_header_->protocol == 6) return true;
            }
		    return false;
	    }

        u_int16_t data_gram_len(void){
            return ntohs(ip_header_->tot_len);
        }

        void set_ip_header(void){
	        ip_header_ = (struct iphdr*)(packet_ + ETHER_HEAD_LEN);
	    }

    private:
	    struct iphdr *ip_header_;	

	    void print_L4_packet_type(uint8_t type){
	        switch(type){
		        case 6:
		            printf("TCP");
		            break;
		        case 17:
		            printf("UDP");
		            break;
		        default:
		            printf("Unknown type");
	        }
	    }
};

class Layer4_packet :public Layer3_packet{
    public:
        int TCP_HDR_LEN = sizeof(tcphdr);
	    void print_tcp_header(void){
	        set_tcp_header();
	        if(is_tcp()){
		        printf("SrcPort#: %d ",sport());
		        printf("DstPort#: %d\n",dport());
	        } 
	    }   

        u_int16_t sport(void){
            set_tcp_header(); 
            return ntohs(tcp_header_->th_sport);
        }

        u_int16_t dport(void){
            set_tcp_header(); 
            return ntohs(tcp_header_->th_dport);
        }

        void set_tcp_header(void){
	        tcp_header_ = (struct tcphdr*)(packet_ 
			    +ETHER_HEAD_LEN
			    +IPv4_HDR_LEN);
	    }

    private:
	    struct tcphdr *tcp_header_;

};

class Layer567_packet :public Layer4_packet{
    public:
        bool is_http(void){
            if(is_tcp()){
                if(sport()==80 /*|| dport()==80*/) return true;
            }
            return false;
        }

        void set_http_top(void){
	        http_top_ = (u_char*)(packet_ 
			    +ETHER_HEAD_LEN
			    +IPv4_HDR_LEN
			    +TCP_HDR_LEN);
	    }

        u_char* http_top(void){
            set_http_top();
            return http_top_;
        }

        u_char* decode_http_header(void){ //return the end of http header
            u_char *now_top = http_top();
            http_hdr_len_ = 0;

            if(is_http_hdr()){
                for(int i=http_payload_len(); 4<=i; i--){
                    if(strncmp("\r\n\r\n", (const char *)now_top, 4)==0){
                        http_hdr_len_ += 4;
                        return now_top+4;
                    }

                    if(strncmp("Content-Length: ", (const char *)now_top, 16)==0){
                        char *cl = (char *)(now_top + 16);
                        char num[32];

                        int i;
                        for(i=0; cl[i]!='\n'; i++){
                            num[i] = cl[i];
                        }
                        content_len_ = atoi(num);
                    }

                    now_top++;
                    http_hdr_len_++;
                }
            }
            return (u_char *)NULL;
        }

        int http_hdr_len(void){
            return http_hdr_len_;
        }

        int content_length(void){
            return content_len_;
        }

        bool is_http_hdr(void){
            set_http_top();
            if(5 <= http_payload_len()){
                if(strncmp("HTTP", (const char *)http_top(), 4) == 0){
                    return true;
                }
            }
            return false;
        }

        int http_payload_len(){
            return data_gram_len()
                    -IPv4_HDR_LEN
                    -TCP_HDR_LEN;
        }

    private:
	    u_char *http_top_;
        int http_hdr_len_;
        int content_len_;
};

class Packet :public Layer567_packet{
    public:
        void set_packet(const u_char *p, pcap_pkthdr h){
            packet_ = p;
            header_ = h;
        }
};

class Capture_packet{
    public:
        Capture_packet& print_all_dev(void){
            printf("All interfaces are shown below\n");

            pcap_if_t *devinfo_p = devinfo_;
            while(devinfo_p->next != NULL){
                printf("Device: %s\n",devinfo_p->name);
                devinfo_p = devinfo_p->next;
            }
            printf("\n");
            return *this;
        }

        Capture_packet& find_dev_info(void){
            if(pcap_findalldevs(&devinfo_, errbuf)==-1){
                printf("%s\n",errbuf);
                exit(0);
            }
            return *this;
        }

        Capture_packet& open_hadler(void){
            printf("Enter the interface name you watch\n");
            char dev_name[128];
            get_string(dev_name);

            handler_ = pcap_open_live(dev_name, 8000, 1, 10, errbuf);
            if(handler_ == NULL){
                printf("%s\n", errbuf);
                exit(0);
            }else{
                printf("Seccess!\n\n");
            }
            return *this;
        }

        Capture_packet& capture(Packet& packet){
            const u_char *p;
            pcap_pkthdr h;
            p = pcap_next(handler_, &h);

            if(p != NULL) packet.set_packet(p, h);
            return *this;
        }

        Capture_packet& capture_http(Packet& packet){
            while(1){
                capture(packet);
                if(packet.is_http()) break;
            }
            return *this;
        }


        ~Capture_packet(void){
            pcap_freealldevs(devinfo_);
            pcap_close(handler_);
        }
    
    private:
        pcap_if_t *devinfo_;
        pcap_t *handler_;
        char errbuf[512];
        class Packet packet_;

        void get_string(char *s){
            int i=0;
            while((s[i] = getchar()) != '\n') i++ ;
            s[i] = '\0';
        }
};

int file_nstr_write(FILE *fp, int n, char *s){ 
    if(n<=0) return 0;
    int i;
    for(i=0; i<n; i++){
        fprintf(fp, "%c" , s[i]);
    }
    return i;
}

int main(void){
    class Packet packet;
    class Capture_packet capture_packet;

    capture_packet.find_dev_info().print_all_dev().open_hadler();

    FILE *fp;
    char filename[32];
    bool flag = false;
    u_char *http_body_start;
    int name_index=0 ,total_len=0;

    char png[8] = {(char)0x89,(char)0x50,(char)0x4E,(char)0x47
                    ,(char)0x0D,(char)0x0A,(char)0x1A,(char)0x0A};
    char jpg[2] = {(char)0xFF, (char)0xD8};

    while(1){

        while(1){  //wait for HTTP header
            if(flag==false)capture_packet.capture_http(packet);
            flag = false;
            http_body_start = packet.decode_http_header();
            if(http_body_start != NULL){
                flag = true;
                break;
            } 
        }

        if( strncmp((const char *)http_body_start, png, 8) == 0){ //png file
            sprintf(filename, "image%d.png",name_index);
            fp = fopen(filename,"wb+");
            name_index++;

        }else if( strncmp((const char *)http_body_start, jpg, 2) == 0 ){ // jpg file
            sprintf(filename, "image%d.jpg",name_index);
            fp = fopen(filename,"wb+");
            name_index++;

        }else{ //http text file
            sprintf(filename, "text%d.html",name_index);
            fp = fopen(filename,"w+");
            name_index++;
        }

        //the first packet
        total_len += file_nstr_write(fp,  
                        packet.http_payload_len()-packet.http_hdr_len(), 
                        (char *)http_body_start);

        while(1){
            if(packet.content_length()<=total_len) break;
            capture_packet.capture_http(packet);
            total_len += 
                file_nstr_write(fp, packet.http_payload_len(), (char *)packet.http_top());
        }

        capture_packet.capture_http(packet);
        if(packet.http_payload_len() == 0){
            break;
        }else{
            flag = true;
        }

        fclose(fp);
        total_len = 0;
    }
    return 0;
}
