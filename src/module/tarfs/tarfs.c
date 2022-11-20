#include <typedefs.h>
#include <vfs.h>
struct ustar {
	char filename[100];
	char mode[8];
	char ownerid[8];
	char groupid[8];

	char size[12];
	char mtime[12];

	char checksum[8];
	char type[1];
	char link[100];

	char ustar[6];
	char version[2];

	char owner[32];
	char group[32];

	char dev_major[8];
	char dev_minor[8];

	char prefix[155];
};

static unsigned int interpret_uid(struct ustar * file) {
	return 
		((file->ownerid[0] - '0') << 18) |
		((file->ownerid[1] - '0') << 15) |
		((file->ownerid[2] - '0') << 12) |
		((file->ownerid[3] - '0') <<  9) |
		((file->ownerid[4] - '0') <<  6) |
		((file->ownerid[5] - '0') <<  3) |
		((file->ownerid[6] - '0') <<  0);
}

static unsigned int interpret_gid(struct ustar * file) {
	return 
		((file->groupid[0] - '0') << 18) |
		((file->groupid[1] - '0') << 15) |
		((file->groupid[2] - '0') << 12) |
		((file->groupid[3] - '0') <<  9) |
		((file->groupid[4] - '0') <<  6) |
		((file->groupid[5] - '0') <<  3) |
		((file->groupid[6] - '0') <<  0);
}

static unsigned int interpret_mode(struct ustar * file) {
	return 
		((file->mode[0] - '0') << 18) |
		((file->mode[1] - '0') << 15) |
		((file->mode[2] - '0') << 12) |
		((file->mode[3] - '0') <<  9) |
		((file->mode[4] - '0') <<  6) |
		((file->mode[5] - '0') <<  3) |
		((file->mode[6] - '0') <<  0);
}

static unsigned int interpret_size(struct ustar * file) {
	return
		((file->size[ 0] - '0') << 30) |
		((file->size[ 1] - '0') << 27) |
		((file->size[ 2] - '0') << 24) |
		((file->size[ 3] - '0') << 21) |
		((file->size[ 4] - '0') << 18) |
		((file->size[ 5] - '0') << 15) |
		((file->size[ 6] - '0') << 12) |
		((file->size[ 7] - '0') <<  9) |
		((file->size[ 8] - '0') <<  6) |
		((file->size[ 9] - '0') <<  3) |
		((file->size[10] - '0') <<  0);
}

static unsigned int round_to_512(unsigned int i) {
	unsigned int t = i % 512;

	if (!t) return i;
	return i + (512 - t);
}
void tarfs_read((struct vfs_node *node,uint32_t offset,uint32_t how,void *buf) {
	printf("TARFS read didn't implemented\n");
}
void tarfs_write((struct vfs_node *node,uint32_t offset,uint32_t how,void *buf) {
	printf("TARFS write didn't implemented\n");
}
void tarfs_open((struct vfs_node *node,bool w,bool r) {
	printf("TARFS open didn't implemented\n");
}
void tarfs_close(struct vfs_node *node) {}
struct vfs_node *tarfs_finddir(struct vfs_node *in,char *name) {
	printf("TARFS finddir didn't implemented\n");
	return NULL;
}
struct dirent *tarfs_readdir(struct vfs_node *dir,uint32_t index) {
	printf("TARFS readdir didn't implemented\n");
	return 0;
}
void tarfs_main() {
	printf("TARFS didn't implemented yet\n");
}
void tarfs_main() {
	printf("TARFS\n");
}
